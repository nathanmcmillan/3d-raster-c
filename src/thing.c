/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

void ThingDoNothing(void *thing) {
    (void)thing;
}

void ThingAddToCells(Thing *thing) {
    float box = thing->box;
    int min_c = (int)(thing->x - box) >> WORLD_CELL_SHIFT;
    int max_c = (int)(thing->x + box) >> WORLD_CELL_SHIFT;
    int min_r = (int)(thing->z - box) >> WORLD_CELL_SHIFT;
    int max_r = (int)(thing->z + box) >> WORLD_CELL_SHIFT;
    if (min_c < 0) min_c = 0;
    if (min_r < 0) min_r = 0;
    if (max_c >= CELL_COLUMNS) max_c = CELL_COLUMNS - 1;
    if (max_r >= CELL_ROWS) max_c = CELL_ROWS - 1;
    for (int r = min_r; r <= max_r; r++) {
        for (int c = min_c; c <= max_c; c++) {
            CellAddThing(&CELLS[c + r * CELL_COLUMNS], thing);
        }
    }
    thing->min_c = min_c;
    thing->max_c = max_c;
    thing->min_r = min_r;
    thing->max_r = max_r;
}

void ThingUpdateSector(Thing *thing, Sector *sector) {
    if (thing->sector == sector)
        return;
    SectorRemoveThing(thing->sector, thing);
    thing->sector = sector;
    SectorAddThing(sector, thing);
}

void ThingInit(Thing *thing) {
    Sector *sector = WorldFindSector(thing->x, thing->z);
    if (sector == NULL) {
        fprintf(stderr, "THING NOT IN SECTOR <%g, %g>\n", thing->x, thing->z);
        exit(1);
    }
    thing->sector = sector;
    SectorAddThing(sector, thing);
    if (thing->y == 0)
        thing->y = sector->floor;
    ThingAddToCells(thing);
    WorldAddThing(thing);
}

void ThingRemoveFromCells(Thing *thing) {
    for (int r = thing->min_r; r <= thing->max_r; r++) {
        for (int c = thing->min_c; c <= thing->max_c; c++) {
            CellRemoveThing(&CELLS[c + r * CELL_COLUMNS], thing);
        }
    }
}

Thing *NewThing(float x, float y, float z) {
    Thing *thing = Calloc(1, sizeof(Thing));
    thing->x = x;
    thing->y = y;
    thing->z = z;
    thing->box = 0.6f;
    thing->update = ThingDoNothing;
    ThingInit(thing);
    return thing;
}

bool thing_collision(Thing *thing, Thing *b) {
    float block = thing->box + b->box;
    return !(fabs(thing->x - b->x) > block || fabs(thing->z - b->z) > block);
}

void thing_resolve_collision(Thing *thing, Thing *b) {
    float block = thing->box + b->box;

    if (fabs(thing->x - b->x) > block || fabs(thing->z - b->z) > block)
        return;

    if (fabs(thing->previous_x - b->x) > fabs(thing->previous_z - b->z)) {
        if (thing->previous_x - b->x < 0) {
            thing->x = b->x - block;
        } else {
            thing->x = b->x + block;
        }
        thing->dx = 0.0f;
    } else {
        if (thing->previous_z - b->z < 0) {
            thing->z = b->z - block;
        } else {
            thing->z = b->z + block;
        }
        thing->dz = 0.0f;
    }
}

void thing_line_collision(Thing *thing, Line *ld) {
    float box = thing->box;

    float vx = ld->b->x - ld->a->x;
    float vz = ld->b->y - ld->a->y;

    float wx = thing->x - ld->a->x;
    float wz = thing->z - ld->a->y;

    float t = (wx * vx + wz * vz) / (vx * vx + vz * vz);

    bool endpoint = false;

    if (t < 0) {
        t = 0;
        endpoint = true;
    } else if (t > 1) {
        t = 1;
        endpoint = true;
    }

    float px = ld->a->x + vx * t;
    float pz = ld->a->y + vz * t;

    px -= thing->x;
    pz -= thing->z;

    if ((px * px + pz * pz) > box * box)
        return;

    bool collision = false;

    if (ld->side_front.middle != LINE_NO_SIDE) {
        collision = true;
    } else {
        if (thing->y + thing->height > ld->front->ceiling || thing->y + 1.0f < ld->front->floor) {
            collision = true;
        }
    }

    if (collision) {
        if (thing->sector == ld->front)
            return;

        float overlap;

        float normal_x;
        float normal_z;

        if (endpoint) {
            float ex = -px;
            float ez = -pz;

            float em = sqrtf(ex * ex + ez * ez);

            ex /= em;
            ez /= em;

            overlap = sqrtf((px + box * ex) * (px + box * ex) + (pz + box * ez) * (pz + box * ez));

            normal_x = ex;
            normal_z = ez;
        } else {
            overlap = sqrtf((px + box * ld->normal.x) * (px + box * ld->normal.x) + (pz + box * ld->normal.y) * (pz + box * ld->normal.y));

            normal_x = ld->normal.x;
            normal_z = ld->normal.y;
        }

        thing->x += normal_x * overlap;
        thing->z += normal_z * overlap;
    }
}

void ThingIntegrate(Thing *thing) {
    if (thing->ground) {
        thing->dx *= RESISTANCE;
        thing->dz *= RESISTANCE;
    }

    if (FLOAT_NOT_ZERO(thing->dx) || FLOAT_NOT_ZERO(thing->dz)) {
        thing->previous_x = thing->x;
        thing->previous_z = thing->z;

        thing->x += thing->dx;
        thing->z += thing->dz;

        ThingRemoveFromCells(thing);

        float box = thing->box;
        int min_c = (int)(thing->x - box) >> WORLD_CELL_SHIFT;
        int max_c = (int)(thing->x + box) >> WORLD_CELL_SHIFT;
        int min_r = (int)(thing->z - box) >> WORLD_CELL_SHIFT;
        int max_r = (int)(thing->z + box) >> WORLD_CELL_SHIFT;

        Set *collided = NewAddressSet();
        Set *collisions = NewAddressSet();

        for (int r = min_r; r <= max_r; r++) {
            for (int c = min_c; c <= max_c; c++) {
                Cell *current_cell = &CELLS[c + r * CELL_COLUMNS];
                for (int i = 0; i < current_cell->thing_count; i++) {
                    Thing *t = current_cell->things[i];

                    if (SetHas(collisions, t))
                        continue;

                    if (thing_collision(thing, t))
                        SetAdd(collided, t);

                    SetAdd(collisions, t);
                }
            }
        }

        SetFree(collisions);

        while (set_not_empty(collided)) {
            Thing *closest = NULL;
            float manhattan = FLT_MAX;

            SetIterator iter = NewSetIterator(collided);
            while (SetIteratorHasNext(&iter)) {
                Thing *b = SetIteratorNext(&iter);
                float distance = fabsf(thing->previous_x - b->x) + fabsf(thing->previous_z - b->z);
                if (distance < manhattan) {
                    manhattan = distance;
                    closest = b;
                }
            }

            thing_resolve_collision(thing, closest);

            set_remove(collided, closest);
        }

        SetFree(collided);

        for (int r = min_r; r <= max_r; r++) {
            for (int c = min_c; c <= max_c; c++) {
                Cell *current_cell = &CELLS[c + r * CELL_COLUMNS];
                for (int i = 0; i < current_cell->line_count; i++)
                    thing_line_collision(thing, current_cell->lines[i]);
            }
        }

        ThingAddToCells(thing);
    }

    if (thing->ground == false || FLOAT_NOT_ZERO(thing->dy)) {
        thing->dy -= GRAVITY;
        thing->y += thing->dy;

        if (thing->y < thing->sector->floor) {
            thing->ground = true;
            thing->dy = 0;
            thing->y = thing->sector->floor;
        } else {
            thing->ground = false;
        }
    }
}
