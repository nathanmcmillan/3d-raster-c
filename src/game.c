/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "state.h"

Game *new_game(Canvas *canvas, Hymn *vm, Input *input, Assets *assets) {
    Game *this = safe_calloc(1, sizeof(Game));
    this->state.canvas = canvas;
    this->state.vm = vm;
    this->state.input = input;
    this->state.assets = assets;
    this->state.update = game_update;
    this->state.draw = game_draw;
    this->world = new_world();
    this->camera = new_camera(8.0);
    return this;
}

static int texture(Assets *assets, String *name) {
    if (strcmp(name, "none")) {
        return -1;
    }
    return assets_paint_name_to_index(assets, name);
}

void game_open(Game *this, String *content) {
    World *world = this->world;
    Assets *assets = this->state.assets;

    world_clear(world);

    MaybeWad parse = wad_parse(content);
    if (parse.error) {
        fprintf(stderr, parse.error);
        exit(1);
    }
    Wad *wad = parse.wad;

    WadArray *map_vecs = wad_get_array_from_object(wad, "vectors");
    WadArray *map_lines = wad_get_array_from_object(wad, "lines");
    WadArray *map_sectors = wad_get_array_from_object(wad, "sectors");

    Array *vecs = new_array(0);
    Array *lines = new_array(0);

    for (usize i = 0; i < map_vecs->length; i++) {
        Wad *point = ((Wad *)map_vecs->items[i]);
        float x = wad_get_float(wad_get_from_object(point, "x"));
        float z = wad_get_float(wad_get_from_object(point, "z"));
        array_push(vecs, new_vec(x, z));
    }

    for (usize i = 0; i < map_lines->length; i++) {
        Wad *line = ((Wad *)map_lines->items[i]);
        int s = wad_get_int(wad_get_from_object(line, "s"));
        int e = wad_get_int(wad_get_from_object(line, "e"));
        Vec *a = array_get(vecs, s);
        Vec *b = array_get(vecs, e);
        array_push(lines, new_line(a, b, LINE_NO_WALL, LINE_NO_WALL, LINE_NO_WALL));
    }

    for (usize i = 0; i < map_sectors->length; i++) {
        Wad *sector = ((Wad *)map_sectors->items[i]);
        float bottom = wad_get_float(wad_get_from_object(sector, "b"));
        float floor = wad_get_float(wad_get_from_object(sector, "f"));
        float ceiling = wad_get_float(wad_get_from_object(sector, "c"));
        float top = wad_get_float(wad_get_from_object(sector, "t"));
        int floor_paint = texture(assets, wad_get_string(wad_get_from_object(sector, "u")));
        int ceiling_paint = texture(assets, wad_get_string(wad_get_from_object(sector, "v")));
        WadArray *vec_ptrs = wad_get_array(wad_get_from_object(sector, "vecs"));
        WadArray *line_ptrs = wad_get_array(wad_get_from_object(sector, "lines"));
        int vec_count = (int)array_size(vec_ptrs);
        int line_count = (int)array_size(line_ptrs);
        Vec **sector_vecs = safe_calloc(vec_count, sizeof(Vec *));
        Line **sector_lines = safe_calloc(line_count, sizeof(Line *));
        for (int v = 0; v < vec_count; v++) {
            sector_vecs[v] = array_get(vecs, wad_get_int((Wad *)vec_ptrs->items[v]));
        }
        for (int d = 0; d < line_count; d++) {
            sector_lines[d] = array_get(lines, wad_get_int((Wad *)line_ptrs->items[d]));
        }
        Sector *sector_object = new_sector(sector_vecs, vec_count, sector_lines, line_count, bottom, floor, ceiling, top, floor_paint, ceiling_paint);
        world_add_sector(world, sector_object);
    }

    world_build(world, lines);

    WadArray *map_things = wad_get_array_from_object(wad, "things");
    if (map_things) {
        for (usize i = 0; i < map_things->length; i++) {
            Wad *thing = ((Wad *)map_things->items[i]);
            float x = wad_get_float(wad_get_from_object(thing, "x"));
            float z = wad_get_float(wad_get_from_object(thing, "z"));
            String *id = wad_get_string(wad_get_from_object(thing, "id"));
            if (strcmp(id, "hero")) {
                this->camera->x = x;
                this->camera->z = z;
                Sector *sector = world_find_sector(world, x, z);
                this->camera->sector = sector != NULL ? sector : world->sectors[0];
            }
        }
    }

    array_delete(vecs);
    wad_delete(wad);
}

void game_update(void *state) {
    Game *this = (Game *)state;

    Input *input = this->state.input;
    Camera *camera = this->camera;

    if (input->move_up) {
        camera->x += 0.1f;
    }

    if (input->move_down) {
        camera->x -= 0.1f;
    }

    if (input->move_left) {
        camera->z += 0.1f;
    }

    if (input->move_right) {
        camera->z -= 0.1f;
    }

    if (input->look_left) {
        camera->ry += 0.1f;
    }

    if (input->look_right) {
        camera->ry -= 0.1f;
    }

    hymn_call(this->state.vm, "update", 0);
}

struct IVec2 {
    i32 x;
    i32 y;
};

typedef struct IVec2 IVec2;

struct Visit {
    Sector *sector;
    i32 sx1;
    i32 sx2;
};

typedef struct Visit Visit;

#define EYE_HEIGHT 6
#define DUCK_HEIGHT 2.5f
#define HEAD_MARGIN 1
#define KNEE_HEIGHT 2

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define CLAMP(a, l, u) MIN(MAX(a, l), u)
#define OVERLAP(a0, a1, b0, b1) (MIN(a0, a1) <= MAX(b0, b1) && MIN(b0, b1) <= MAX(a0, a1))
#define INTERSECT_BOX(x0, y0, x1, y1, x2, y2, x3, y3) (OVERLAP(x0, x1, x2, x3) && OVERLAP(y0, y1, y2, y3))
#define CROSS(x0, y0, x1, y1) (x0 * y1 - x1 * y0)
#define POINT_SIDE(px, py, x0, y0, x1, y1) CROSS(x1 - x0, y1 - y0, px - x0, py - y0)

Vec2 intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    return (Vec2){
        CROSS(CROSS(x1, y1, x2, y2), x1 - x2, CROSS(x3, y3, x4, y4), x3 - x4) / CROSS(x1 - x2, y1 - y2, x3 - x4, y3 - y4),
        CROSS(CROSS(x1, y1, x2, y2), y1 - y2, CROSS(x3, y3, x4, y4), x3 - x4) / CROSS(x1 - x2, y1 - y2, x3 - x4, y3 - y4),
    };
}

void draw_vertical_line(i32 width, i32 height, u32 *pixels, i32 x, i32 y1, i32 y2, u32 top, u32 middle, u32 bottom) {
    height--;
    y1 = CLAMP(y1, 0, height);
    y2 = CLAMP(y2, 0, height);
    if (y2 == y1) {
        pixels[y1 * width + x] = middle;
    } else if (y2 > y1) {
        pixels[y1 * width + x] = top;
        for (i32 y = y1 + 1; y < y2; y++) {
            pixels[y * width + x] = middle;
        }
        pixels[y2 * width + x] = bottom;
    }
}

void game_draw(void *state) {
    Game *this = (Game *)state;

    Canvas *canvas = this->state.canvas;
    i32 width = canvas->width;
    i32 height = canvas->height;
    u32 *pixels = canvas->pixels;

    World *world = this->world;
    Camera *camera = this->camera;

    canvas_clear(canvas);

    canvas_rectangle(canvas, rgb(255, 255, 0), 10, 60, 42, 92);

    // float perspective[16];
    // float view[16];
    // float projection[16];

    // float fov = 60.0f;
    // float ratio = (float)width / (float)height;
    // float near = 0.01f;
    // float far = 100.0f;

    float camera_x = camera->x;
    // float camera_y = camera->y;
    float camera_z = camera->z;

    float camera_sin = sinf(camera->ry);
    float camera_cos = cosf(camera->ry);

    Sector *find = world_find_sector(world, camera_x, camera_z);
    if (find) this->camera->sector = find;

    float horizontal_fov = 0.73f * (float)width;
    float vertical_fov = 0.2f * (float)width;

    const float near_z = 1e-4f;
    const float far_z = 5.0f;
    const float near_side = 1e-5f;
    const float far_side = 20.0f;

    IVec2 *remaining_y = safe_calloc(width, sizeof(IVec2));
    for (i32 i = 0; i < width; i++) {
        remaining_y[i].y = height - 1;
    }

#define QUEUE_COUNT 32
    Visit queue[QUEUE_COUNT];

    Visit *head = queue + 1;
    Visit *tail = queue;

    *tail = (Visit){camera->sector, 0, width - 1};
    printf("X\n");
    while (true) {
        printf("Y\n");
        Visit current = *tail;
        Sector *sector = current.sector;

        if (++tail == queue + QUEUE_COUNT) {
            tail = queue;
        }

        for (int e = 0; e < sector->line_count; e++) {
            Line *line = sector->lines[e];
            Vec *a = line->a;
            Vec *b = line->b;
            // transform to camera view
            float vx1 = a->x - camera_x;
            float vy1 = a->y - camera_z;
            float vx2 = b->x - camera_x;
            float vy2 = b->y - camera_z;
            // rotate around camera view
            float tx1 = vx1 * camera_sin - vy1 * camera_cos;
            float tz1 = vx1 * camera_cos + vy1 * camera_sin;
            float tx2 = vx2 * camera_sin - vy2 * camera_cos;
            float tz2 = vx2 * camera_cos + vy2 * camera_sin;
            // is partially in front of camera
            if (tz1 <= 0 && tz2 <= 0) {
                continue;
            }
            // clip against camera view frustum
            if (tz1 <= 0 || tz2 <= 0) {
                Vec2 i1 = intersect(tx1, tz1, tx2, tz2, -near_side, near_z, -far_side, far_z);
                Vec2 i2 = intersect(tx1, tz1, tx2, tz2, near_side, near_z, far_side, far_z);
                if (tz1 < near_z) {
                    if (i1.y > 0) {
                        tx1 = i1.x;
                        tz1 = i1.y;
                    } else {
                        tx1 = i2.x;
                        tz1 = i2.y;
                    }
                }
                if (tz2 < near_z) {
                    if (i1.y > 0) {
                        tx2 = i1.x;
                        tz2 = i1.y;
                    } else {
                        tx2 = i2.x;
                        tz2 = i2.y;
                    }
                }
            }
            // perspective transformation
            float xscale1 = horizontal_fov / tz1;
            float xscale2 = horizontal_fov / tz2;
            float yscale1 = vertical_fov / tz1;
            float yscale2 = vertical_fov / tz2;
            i32 x1 = width / 2 - (i32)(tx1 * xscale1);
            i32 x2 = width / 2 - (i32)(tx2 * xscale2);
            // is visible
            if (x1 >= x2 || x2 < current.sx1 || x1 > current.sx2) {
                continue;
            }
            float yceil = sector->ceiling - camera->y;
            float yfloor = sector->floor - camera->y;
            // project floor and ceiling to screen coordinates
            i32 y1a = height / 2 - (i32)(yceil * yscale1);
            i32 y1b = height / 2 - (i32)(yfloor * yscale1);
            i32 y2a = height / 2 - (i32)(yceil * yscale2);
            i32 y2b = height / 2 - (i32)(yfloor * yscale2);
            // render
            i32 beginx = MAX(x1, current.sx1);
            i32 endx = MIN(x2, current.sx2);
            for (i32 x = beginx; x <= endx; x++) {
                i32 ya = (x - x1) * (y2a - y1a) / (x2 - x1) + y1a;
                i32 cya = CLAMP(ya, remaining_y[x].x, remaining_y[x].y);
                i32 yb = (x - x1) * (y2b - y1b) / (x2 - x1) + y1b;
                i32 cyb = CLAMP(yb, remaining_y[x].x, remaining_y[x].y);
                draw_vertical_line(width, height, pixels, x, remaining_y[x].x, cya - 1, 0x111111, 0x222222, 0x111111);
                draw_vertical_line(width, height, pixels, x, cyb + 1, remaining_y[x].y, 0x0000ff, 0x0000aa, 0x0000ff);
                // if neighbor ...
                draw_vertical_line(width, height, pixels, x, cya, cyb, 0x0, 0xaaaaaa, 0x0);
            }

            // if neighbor ...
        }

        if (tail == head) {
            break;
        }
    }

    printf("Z\n");

    hymn_call(this->state.vm, "draw", 0);
}

void game_delete(Game *this) {
    free(this);
}
