/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

static Set *DEAD = NULL;
static Set *SEARCH_DONE = NULL;
static Array *NEIGHBOR_QUEUE = NULL;

void SectorsInit() {
    DEAD = NewAddressSet();
    SEARCH_DONE = NewAddressSet();
    NEIGHBOR_QUEUE = NewArray(0);
}

void SectorsFree() {
    SetFree(DEAD);
    SetFree(SEARCH_DONE);
    ArrayFree(NEIGHBOR_QUEUE);
}

Line *NewLine(int id, Vec *a, Vec *b, Side front, Side back) {
    Line *line = Calloc(1, sizeof(Line));
    line->id = id;
    line->a = a;
    line->b = b;
    line->normal = vec_normal(a, b);
    line->side_front = front;
    line->side_back = back;
    return line;
}

Sector *NewSector(int id, Line **lines, int line_count, float floor, float ceiling, int floor_image, int ceiling_image) {
    Sector *sector = Calloc(1, sizeof(Sector));
    sector->id = id;
    sector->vecs = Calloc(line_count, sizeof(Vec *));
    sector->vec_count = line_count;
    for (int v = 0; v < line_count; v++)
        sector->vecs[v] = lines[v]->a;
    sector->lines = lines;
    sector->line_count = line_count;
    sector->floor = floor;
    sector->ceiling = ceiling;
    sector->floor_image = floor_image;
    sector->ceiling_image = ceiling_image;
    return sector;
}

bool SectorContains(Sector *sector, float x, float z) {
    bool odd = false;
    Vec **vecs = sector->vecs;
    int count = sector->vec_count;
    int n = count - 1;
    for (int v = 0; v < count; v++) {
        Vec *a = vecs[v];
        Vec *b = vecs[n];
        if ((a->y > z) != (b->y > z)) {
            float value = (b->x - a->x) * (z - a->y) / (b->y - a->y) + a->x;
            if (x < value)
                odd = !odd;
        }
        n = v;
    }
    return odd;
}

Sector *SectorFind(Sector *sector, float x, float z) {
    Sector **inside = sector->inside;
    int i = sector->inside_count;
    while (i-- != 0) {
        Sector *s = inside[i];
        if (SectorContains(s, x, z))
            return SectorFind(s, x, z);
    }
    return sector;
}

Sector *SectorSearchFor(Sector *sector, float x, float z) {
    if (SectorContains(sector, x, z))
        return SectorFind(sector, x, z);
    SetClear(SEARCH_DONE);
    SetAdd(SEARCH_DONE, sector);
    ArrayClear(NEIGHBOR_QUEUE);
    for (int n = 0; n < sector->neighbor_count; n++)
        ArrayPush(NEIGHBOR_QUEUE, sector->neighbors[n]);
    while (ArrayNotEmpty(NEIGHBOR_QUEUE)) {
        Sector *current = ArrayGet(NEIGHBOR_QUEUE, 0);
        if (SectorContains(current, x, z))
            return SectorFind(current, x, z);
        ArrayRemoveAt(NEIGHBOR_QUEUE, 0);
        SetAdd(SEARCH_DONE, current);
        for (int n = 0; n < current->neighbor_count; n++) {
            Sector *neighbor = current->neighbors[n];
            if (SetHas(SEARCH_DONE, neighbor) || ArrayFindAddress(NEIGHBOR_QUEUE, neighbor))
                continue;
            ArrayPush(NEIGHBOR_QUEUE, neighbor);
        }
    }
    return NULL;
}

bool SectorHasFloor(Sector *sector) {
    return sector->floor_image != SECTOR_NO_SURFACE;
}

bool SectorHasCeiling(Sector *sector) {
    return sector->ceiling_image != SECTOR_NO_SURFACE;
}

static void SectorAddInside(Sector *sector, Sector *inside) {
    if (sector->inside_count == 0)
        sector->inside = Malloc(sizeof(Sector *));
    else
        sector->inside = Realloc(sector->inside, (sector->inside_count + 1) * sizeof(Sector *));
    sector->inside[sector->inside_count] = inside;
    sector->inside_count++;
}

static void SectorDeleteInside(Sector *in) {
    for (int i = 0; i < in->inside_count; i++) {
        Sector *inside = in->inside[i];
        SetAdd(DEAD, inside);
        SectorDeleteInside(inside);
    }
}

void SectorInsideOutside(Sector **sectors, int sector_count) {
    for (int x = 0; x < sector_count; x++) {
        Sector *sector = sectors[x];
        Vec **sector_vecs = sector->vecs;
        int sector_vec_count = sector->vec_count;
        for (int y = 0; y < sector_count; y++) {
            if (x == y)
                continue;
            Sector *other = sectors[y];
            Vec **other_vecs = other->vecs;
            int other_vec_count = other->vec_count;
            int inside = 0;
            int outside = 0;
            for (int v = 0; v < other_vec_count; v++) {
                Vec *vec = other_vecs[v];
                bool shared = false;
                for (int c = 0; c < sector_vec_count; c++) {
                    if (sector_vecs[c] == vec) {
                        shared = true;
                        break;
                    }
                }
                if (shared)
                    continue;
                if (SectorContains(sector, vec->x, vec->y))
                    inside++;
                else
                    outside++;
            }
            if (outside == 0 && inside > 0)
                SectorAddInside(sector, other);
        }
    }
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        Sector **inside = sector->inside;
        int inside_count = sector->inside_count;
        SetClear(DEAD);
        for (int i = 0; i < inside_count; i++) {
            SectorDeleteInside(inside[i]);
        }
        SetIterator iterator = NewSetIterator(DEAD);
        while (SetIteratorHasNext(&iterator)) {
            Sector *delete = SetIteratorNext(&iterator);
            for (int i = 0; i < inside_count; i++) {
                if (inside[i] == delete) {
                    inside_count--;
                    inside[i] = inside[inside_count];
                    break;
                }
            }
        }
        if (inside_count != sector->inside_count) {
            sector->inside_count = inside_count;
            sector->inside = Realloc(sector->inside, inside_count * sizeof(Sector *));
        }
        for (int i = 0; i < inside_count; i++) {
            sector->inside[i]->outside = sector;
        }
    }
}

static void SectorLineDirection(Sector *sector, Line *line) {
    Vec *a = line->a;
    Vec **vecs = sector->vecs;
    int vector_count = sector->vec_count;
    for (int v = 0; v < vector_count; v++) {
        Vec *vec = vecs[v];
        if (vec != a)
            continue;
        v++;
        if (v == vector_count)
            v = 0;
        if (vecs[v] == line->b)
            line->front = sector;
        else
            line->back = sector;
        return;
    }
}

static bool SectorHasNeighbor(Sector *sector, Sector *other) {
    Sector **neighbors = sector->neighbors;
    int count = sector->neighbor_count;
    for (int n = 0; n < count; n++) {
        if (neighbors[n] == other)
            return true;
    }
    return false;
}

static void SectorAddNeighbor(Sector *sector, Sector *neighbor) {
    if (sector->neighbor_count == 0)
        sector->neighbors = Malloc(sizeof(Sector *));
    else
        sector->neighbors = Realloc(sector->neighbors, (sector->neighbor_count + 1) * sizeof(Sector *));
    sector->neighbors[sector->neighbor_count] = neighbor;
    sector->neighbor_count++;
}

void SectorNeighbors(Sector **sectors, int sector_count, Line **lines, int line_count) {
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        for (int w = 0; w < sector->line_count; w++)
            SectorLineDirection(sector, sector->lines[w]);
    }
    for (int w = 0; w < line_count; w++) {
        Line *line = lines[w];
        Sector *front = line->front;
        Sector *back = line->back;
        if (front != NULL && back != NULL) {
            if (!SectorHasNeighbor(front, back))
                SectorAddNeighbor(front, back);
            if (!SectorHasNeighbor(back, front))
                SectorAddNeighbor(back, front);
        }
    }
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        Sector *outside = sector->outside;
        if (outside == NULL)
            continue;
        if (!SectorHasNeighbor(sector, outside))
            SectorAddNeighbor(sector, outside);
        if (!SectorHasNeighbor(outside, sector))
            SectorAddNeighbor(outside, sector);
        for (int w = 0; w < sector->line_count; w++) {
            Line *line = sector->lines[w];
            if (line->front == NULL)
                line->front = outside;
            else if (line->back == NULL)
                line->back = outside;
        }
    }
}

void SectorAddThing(Sector *sector, Thing *thing) {
    if (sector->thing_capacity == 0) {
        sector->things = Malloc(sizeof(Thing *));
        sector->things[0] = thing;
        sector->thing_capacity = 1;
        sector->thing_count = 1;
        return;
    }
    if (sector->thing_count == sector->thing_capacity) {
        sector->thing_capacity += 8;
        sector->things = Realloc(sector->things, sector->thing_capacity * sizeof(Thing *));
    }
    sector->things[sector->thing_count] = thing;
    sector->thing_count++;
}

void SectorRemoveThing(Sector *sector, Thing *thing) {
    int count = sector->thing_count;
    Thing **things = sector->things;
    for (int i = 0; i < count; i++) {
        if (things[i] == thing) {
            things[i] = things[count - 1];
            sector->thing_count--;
            return;
        }
    }
}
