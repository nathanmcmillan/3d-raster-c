/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "triangulate.h"

typedef struct PolygonVertex PolygonVertex;

struct PolygonVertex {
    int index;
    bool merge;
    bool perimeter;
    Array *last;
    Array *next;
    Vec *point;
};

static PolygonVertex *polygon_vertex_init(Vec *v) {
    PolygonVertex *p = safe_calloc(1, sizeof(PolygonVertex));
    p->point = v;
    p->last = new_array(0);
    p->next = new_array(0);
    return p;
}

static double calc_angle(Vec *a, Vec *b) {
    double angle = atan2(a->y - b->y, a->x - b->x);
    if (angle < 0) {
        angle += MATH_TAU;
    }
    return angle;
}

static double interior_angle(Vec *a, Vec *b, Vec *c) {
    double angle_1 = atan2(a->y - b->y, a->x - b->x);
    double angle_2 = atan2(b->y - c->y, b->x - c->x);
    double interior = angle_2 - angle_1;
    if (interior < 0) {
        interior += MATH_TAU;
    }
    return interior;
}

static bool find_vertex(void *item, void *has) {
    Vec *original = ((PolygonVertex *)item)->point;
    Vec *v = has;
    return (original->x == v->x) and (v->y == original->y);
}

static int compare_vertex(void *item, void *existing) {
    Vec *i = ((PolygonVertex *)item)->point;
    Vec *e = ((PolygonVertex *)existing)->point;
    if (i->y < e->y or (i->y == e->y and i->x > e->x)) {
        return 1;
    }
    return -1;
}

static void clean_population(Array *points) {

    Array *remaining = new_array_copy(points);

    while (array_not_empty(remaining)) {

        PolygonVertex *start = remaining->items[0];
        PolygonVertex *current = start;

        Array *temp = new_array(0);
        Array *dead = new_array(0);
        Array *pending = new_array(0);

        do {
            current->perimeter = true;

            array_remove(remaining, current);

            while (array_size(current->next) != 1) {

                void *pv = current->next->items[1];
                if (!array_find(pending, find_address, pv)) {
                    array_push(pending, pv);
                }
                array_remove_index(current->next, 1);
            }

            while (array_size(current->last) != 1) {
                array_remove_index(current->last, 1);
            }

            current = current->next->items[0];

        } while (current != start);

        while (array_not_empty(pending)) {

            for (unsigned int i = 0; i < pending->length; i++) {
                PolygonVertex *pv = pending->items[i];
                array_push(dead, pv);

                for (unsigned int k = 0; k < pv->next->length; k++) {
                    PolygonVertex *n = pv->next->items[k];

                    if (!n->perimeter) {
                        if (!array_find(pending, find_address, n) and !array_find(temp, find_address, n) and !array_find(dead, find_address, n)) {
                            array_push(temp, n);
                        }
                    }
                }
            }

            array_clear(pending);

            for (unsigned int i = 0; i < temp->length; i++) {
                void *pv = temp->items[i];
                if (!array_find(pending, find_address, pv)) {
                    array_push(pending, pv);
                }
            }

            array_clear(temp);
        }

        for (unsigned int i = 0; i < dead->length; i++) {
            PolygonVertex *pv = dead->items[i];
            array_remove(remaining, pv);
            array_remove(points, pv);
        }
    }
}

static void populate_links(Array *points, Sector *sec, bool clockwise) {
    Vec **vecs = sec->vecs;
    int vec_count = sec->vec_count;

    for (int i = 0; i < vec_count; i++) {
        PolygonVertex *original = array_find(points, find_vertex, vecs[i]);

        PolygonVertex *last = NULL;
        PolygonVertex *next = NULL;

        if (clockwise) {
            if (i == 0) {
                last = array_find(points, find_vertex, vecs[vec_count - 1]);
            } else {
                last = array_find(points, find_vertex, vecs[i - 1]);
            }

            if (i == vec_count - 1) {
                next = array_find(points, find_vertex, vecs[0]);
            } else {
                next = array_find(points, find_vertex, vecs[i + 1]);
            }

        } else {
            if (i == 0) {
                next = array_find(points, find_vertex, vecs[vec_count - 1]);
            } else {
                next = array_find(points, find_vertex, vecs[i - 1]);
            }

            if (i == vec_count - 1) {
                last = array_find(points, find_vertex, vecs[0]);
            } else {
                last = array_find(points, find_vertex, vecs[i + 1]);
            }
        }

        if (array_is_empty(original->last)) {
            array_push(original->last, last);
        } else {
            PolygonVertex *using_last = original->last->items[0];

            double angle = calc_angle(using_last->point, original->point);

            if (calc_angle(last->point, original->point) < angle) {
                array_insert(original->last, 0, last);
            }
        }

        if (array_is_empty(original->next)) {
            array_push(original->next, next);
        } else {
            PolygonVertex *using_next = original->next->items[0];

            double angle = calc_angle(using_next->point, original->point);

            if (calc_angle(next->point, original->point) < angle) {
                array_insert(original->next, 0, next);
            }
        }
    }
}

static void populate_with_vectors(Array *points, Sector *sec) {

    Vec **vecs = sec->vecs;
    int vec_count = sec->vec_count;

    for (int i = 0; i < vec_count; i++) {
        Vec *v = vecs[i];

        PolygonVertex *original = array_find(points, find_vertex, v);

        if (original == NULL) {
            PolygonVertex *vertex = polygon_vertex_init(v);
            array_insert_sort(points, compare_vertex, vertex);
        }
    }
}

static Array *populate(Sector *sec, bool floor) {

    Array *points = new_array(0);

    Sector **inside = sec->inside;
    int inside_count = sec->inside_count;

    for (int i = 0; i < inside_count; i++) {
        Sector *inner = inside[i];
        if (floor) {
            if (sector_has_floor(inner) == false) {
                continue;
            }
        } else {
            if (sector_has_ceiling(inner) == false) {
                continue;
            }
        }

        populate_with_vectors(points, inner);
    }

    for (int i = 0; i < inside_count; i++) {
        Sector *inner = inside[i];
        if (floor) {
            if (sector_has_floor(inner) == false) {
                continue;
            }
        } else {
            if (sector_has_ceiling(inner) == false) {
                continue;
            }
        }

        populate_links(points, inner, false);
    }

    clean_population(points);

    populate_with_vectors(points, sec);
    populate_links(points, sec, true);

    for (unsigned int i = 0; i < points->length; i++) {
        ((PolygonVertex *)points->items[i])->index = i;
    }

    return points;
}

static bool triangle_contains(Vec **tri, float x, float y) {
    bool odd = false;
    int j = 2;
    for (int i = 0; i < 3; i++) {
        Vec *vi = tri[i];
        Vec *vj = tri[j];

        if ((vi->y > y) != (vj->y > y)) {
            float value = (vj->x - vi->x) * (y - vi->y) / (vj->y - vi->y) + vi->x;
            if (x < value) {
                odd = !odd;
            }
        }

        j = i;
    }
    return odd;
}

static bool valid_polygon(Array *poly_vertices, Vec *a, Vec *b) {

    for (unsigned int i = 0; i < poly_vertices->length; i++) {
        PolygonVertex *p = poly_vertices->items[i];

        Vec *c = p->point;
        Vec *d = ((PolygonVertex *)p->last->items[0])->point;

        if (a != c and a != d and b != c and b != d and vec_intersect(a, b, c, d)) {
            return false;
        }
    }

    return true;
}

static bool valid(Array *vecs, Vec *a, Vec *b, Vec *c) {

    if (interior_angle(a, b, c) > MATH_PI) {
        return false;
    }

    Vec *tri[3] = {a, b, c};

    for (unsigned int i = 0; i < vecs->length; i++) {
        Vec *p = vecs->items[i];
        if (p == a or p == b or p == c) {
            continue;
        }
        if (triangle_contains(tri, p->x, p->y)) {
            return false;
        }
    }

    return true;
}

static void clip(Array *vecs, Sector *sec, bool floor, Array *triangles, float scale) {
    unsigned int i = 0;
    unsigned int size = vecs->length;
    while (size > 3) {

        int minus = i - 1;
        if (minus == -1) {
            minus += size;
        }

        Vec *last = vecs->items[minus];
        Vec *pos = vecs->items[i];
        Vec *next = vecs->items[(i + 1) % size];

        if (valid(vecs, last, pos, next)) {

            Triangle *tri;
            if (floor) {
                Vec a = vec_of(last);
                Vec b = vec_of(pos);
                Vec c = vec_of(next);
                tri = new_triangle(sec->floor, sec->floor_paint, a, b, c, floor, scale);
            } else {
                Vec a = vec_of(next);
                Vec b = vec_of(pos);
                Vec c = vec_of(last);
                tri = new_triangle(sec->ceiling, sec->floor_paint, a, b, c, floor, scale);
            }

            array_push(triangles, tri);

            array_remove_index(vecs, i);
            size--;

        } else {
            i++;
        }

        if (i == size) {
            i = 0;
        }
    }

    Triangle *tri;
    if (floor) {
        Vec a = vec_of(vecs->items[0]);
        Vec b = vec_of(vecs->items[1]);
        Vec c = vec_of(vecs->items[2]);
        tri = new_triangle(sec->floor, sec->floor_paint, a, b, c, floor, scale);
    } else {
        Vec a = vec_of(vecs->items[2]);
        Vec b = vec_of(vecs->items[1]);
        Vec c = vec_of(vecs->items[0]);
        tri = new_triangle(sec->ceiling, sec->floor_paint, a, b, c, floor, scale);
    }
    array_push(triangles, tri);
}

static Array *classify(Array *points) {
    Array *start = new_array(0);
    Array *merge = new_array(0);
    Array *split = new_array(0);

    for (unsigned int i = 0; i < points->length; i++) {
        PolygonVertex *pos = points->items[i];
        PolygonVertex *pre = pos->last->items[0];
        PolygonVertex *nex = pos->next->items[0];

        bool reflex = interior_angle(pre->point, pos->point, nex->point) > MATH_PI;
        bool both_above = pre->point->y < pos->point->y and nex->point->y <= pos->point->y;
        bool both_below = pre->point->y >= pos->point->y and nex->point->y >= pos->point->y;
        bool collinear = nex->point->y == pos->point->y;

        if (both_above and reflex) {
#ifdef TRIANGULATE_DEBUG
            printf("classify start: %f, %f\n", pos->point->x, pos->point->y);
#endif
            array_push(start, pos);
        } else if (both_above and !reflex) {
            if (!collinear) {
#ifdef TRIANGULATE_DEBUG
                printf("classify split: %f, %f\n", pos->point->x, pos->point->y);
#endif
                array_push(split, pos);
            }
        } else if (both_below and !reflex) {
            if (!collinear) {
#ifdef TRIANGULATE_DEBUG
                printf("classify merge: %f, %f\n", pos->point->x, pos->point->y);
#endif
                array_push(merge, pos);
            }
        }
    }

    for (unsigned int i = 0; i < merge->length; i++) {
        PolygonVertex *p = merge->items[i];

        unsigned int k;
        for (k = p->index + 1; k < points->length; k++) {
            PolygonVertex *diagonal = points->items[k];
            if (valid_polygon(points, p->point, diagonal->point)) {
                break;
            }
        }

        PolygonVertex *diagonal = points->items[k];

#ifdef TRIANGULATE_DEBUG
        printf("adding merge diangonal from (%f, %f) to (%f, %f)\n", p->point->x, p->point->y, diagonal->point->x, diagonal->point->y);
#endif

        p->merge = true;

        array_push(p->next, diagonal);
        array_push(p->last, diagonal);

        array_push(diagonal->next, p);
        array_push(diagonal->last, p);
    }

    for (unsigned int i = 0; i < split->length; i++) {
        PolygonVertex *p = split->items[i];

        int k;
        for (k = p->index - 1; k >= 0; k--) {
            PolygonVertex *diagonal = points->items[k];
            if (valid_polygon(points, p->point, diagonal->point)) {
                break;
            }
        }

        PolygonVertex *diagonal = points->items[k];

        if (diagonal->merge) {
#ifdef TRIANGULATE_DEBUG
            printf("split using same diagonal as merge (%f, %f) to (%f, %f)\n", p->point->x, p->point->y, diagonal->point->x, diagonal->point->y);
#endif
            continue;
        }

#ifdef TRIANGULATE_DEBUG
        printf("adding split diagonal from (%f, %f) to (%f, %f)\n", p->point->x, p->point->y, diagonal->point->x, diagonal->point->y);
#endif

        array_push(start, diagonal);

        array_push(p->next, diagonal);
        array_push(p->last, diagonal);

        array_push(diagonal->next, p);
        array_push(diagonal->last, p);
    }

    array_delete(merge);
    array_delete(split);

    return start;
}

static void iterate_clip(Array *monotone, Sector *sec, bool floor, Array *triangles, float scale) {
    for (unsigned int i = 0; i < monotone->length; i++) {
        Array *vecs = new_array(0);
        PolygonVertex *ini = monotone->items[i];
        PolygonVertex *nex = ini->next->items[0];
        PolygonVertex *pos = ini;
        do {
            array_push(vecs, vec_copy(pos->point));
            PolygonVertex *pre = NULL;
            double angle = DBL_MAX;
            for (unsigned int j = 0; j < pos->last->length; j++) {
                PolygonVertex *test = pos->last->items[j];
                Vec *a = nex->point;
                Vec *b = pos->point;
                Vec *c = test->point;
                // double interior = interior_angle(a, b, c);
                // why atan2(x, y) not atan2(y, x)

                double angle1 = atan2(a->x - b->x, a->y - b->y);
                double angle2 = atan2(b->x - c->x, b->y - c->y);

                double interior = angle2 - angle1;

                if (interior < 0) {
                    interior += MATH_PI * 2;
                }
                //

                interior += MATH_PI;
                if (interior > MATH_TAU) {
                    interior -= MATH_TAU;
                }
                if (interior < angle) {
                    pre = test;
                    angle = interior;
                }
            }

            array_remove(pos->next, nex);
            array_remove(pos->last, pre);

            nex = pos;
            pos = pre;

        } while (pos != ini);

        clip(vecs, sec, floor, triangles, scale);
    }
}

static void build(Sector *sec, bool floor, Array *triangles, float scale) {
    if (floor) {
        if (sector_has_floor(sec) == false) {
            return;
        }
    } else {
        if (sector_has_ceiling(sec) == false) {
            return;
        }
    }

    Array *points = populate(sec, floor);

#ifdef TRIANGULATE_DEBUG
    printf("points:\n");
    for (unsigned int i = 0; i < points->length; i++) {
        PolygonVertex *vert = points->items[i];
        printf("  (%d) %f, %f\n", vert->index, vert->point->x, vert->point->y);
    }
#endif

    Array *monotone = classify(points);

#ifdef TRIANGULATE_DEBUG
    printf("monotone count %d\n", array_size(monotone));
#endif

    iterate_clip(monotone, sec, floor, triangles, scale);

#ifdef TRIANGULATE_DEBUG
    printf("Triangle count %d\n", array_size(triangles));
#endif

    array_delete(points);
    array_delete(monotone);
}

void triangulate_sector(Sector *sec, float scale) {
    Array *ls = new_array(0);
    build(sec, true, ls, scale);
    build(sec, false, ls, scale);
    sec->triangles = (Triangle **)array_copy_items(ls);
    sec->triangle_count = ls->length;
    array_delete(ls);
}
