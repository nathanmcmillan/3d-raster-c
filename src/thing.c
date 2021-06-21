/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

const float gravity = 0.028;
const float wind_resistance = 0.88;

unsigned int thing_unique_id = 0;

void thing_remove_from_cells(Thing *this) {
    World *map = this->map;
    for (int r = this->r_min; r <= this->r_max; r++)
        for (int c = this->c_min; c <= this->c_max; c++)
            cell_remove_thing(&map->cells[c + r * map->columns], this);
}

void thing_add_to_cells(Thing *this) {
    float box = this->box;
    int c_min = (int)(this->x - box) >> WORLD_CELL_SHIFT;
    int c_max = (int)(this->x + box) >> WORLD_CELL_SHIFT;
    int r_min = (int)(this->z - box) >> WORLD_CELL_SHIFT;
    int r_max = (int)(this->z + box) >> WORLD_CELL_SHIFT;

    World *map = this->map;
    for (int r = r_min; r <= r_max; r++)
        for (int c = c_min; c <= c_max; c++)
            cell_add_thing(&map->cells[c + r * map->columns], this);

    this->c_min = c_min;
    this->c_max = c_max;
    this->r_min = r_min;
    this->r_max = r_max;
}

bool thing_collision(Thing *this, Thing *b) {
    float block = this->box + b->box;
    return !(fabs(this->x - b->x) > block or fabs(this->z - b->z) > block);
}

void thing_resolve_collision(Thing *this, Thing *b) {
    float block = this->box + b->box;

    if (fabs(this->x - b->x) > block or fabs(this->z - b->z) > block)
        return;

    if (fabs(this->previous_x - b->x) > fabs(this->previous_z - b->z)) {
        if (this->previous_x - b->x < 0) {
            this->x = b->x - block;
        } else {
            this->x = b->x + block;
        }
        this->dx = 0.0f;
    } else {
        if (this->previous_z - b->z < 0) {
            this->z = b->z - block;
        } else {
            this->z = b->z + block;
        }
        this->dz = 0.0f;
    }
}

void thing_line_collision(Thing *this, Line *ld) {

    float box = this->box;

    float vx = ld->b->x - ld->a->x;
    float vz = ld->b->y - ld->a->y;

    float wx = this->x - ld->a->x;
    float wz = this->z - ld->a->y;

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

    px -= this->x;
    pz -= this->z;

    if ((px * px + pz * pz) > box * box)
        return;

    bool collision = false;

    if (ld->middle != NULL) {
        collision = true;
    } else {
        if (this->y + this->height > ld->plus->ceiling or this->y + 1.0f < ld->plus->floor) {
            collision = true;
        }
    }

    if (collision) {
        if (this->sec == ld->plus)
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

        this->x += normal_x * overlap;
        this->z += normal_z * overlap;
    }
}

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#endif

#ifdef _MSC_VER
#define UNUSED
#endif

void thing_nop_update(UNUSED void *this) {
}

void thing_standard_update(Thing *this) {
    if (this->ground) {
        this->dx *= wind_resistance;
        this->dz *= wind_resistance;
    }

    if (FLOAT_NOT_ZERO(this->dx) or FLOAT_NOT_ZERO(this->dz)) {
        this->previous_x = this->x;
        this->previous_z = this->z;

        this->x += this->dx;
        this->z += this->dz;

        thing_remove_from_cells(this);

        float box = this->box;
        int c_min = (int)(this->x - box) >> WORLD_CELL_SHIFT;
        int c_max = (int)(this->x + box) >> WORLD_CELL_SHIFT;
        int r_min = (int)(this->z - box) >> WORLD_CELL_SHIFT;
        int r_max = (int)(this->z + box) >> WORLD_CELL_SHIFT;

        Set *collided = new_set(set_address_equal, set_address_hashcode);
        Set *collisions = new_set(set_address_equal, set_address_hashcode);

        World *map = this->map;

        for (int r = r_min; r <= r_max; r++) {
            for (int c = c_min; c <= c_max; c++) {
                Cell *current_cell = &map->cells[c + r * map->columns];
                for (int i = 0; i < current_cell->thing_count; i++) {
                    Thing *t = current_cell->things[i];

                    if (set_has(collisions, t))
                        continue;

                    if (thing_collision(this, t))
                        set_add(collided, t);

                    set_add(collisions, t);
                }
            }
        }

        set_delete(collisions);

        while (set_not_empty(collided)) {
            Thing *closest = NULL;
            float manhattan = FLT_MAX;

            SetIterator iter = new_set_iterator(collided);
            while (set_iterator_has_next(&iter)) {
                Thing *b = set_iterator_next(&iter);
                float distance = fabs(this->previous_x - b->x) + fabs(this->previous_z - b->z);
                if (distance < manhattan) {
                    manhattan = distance;
                    closest = b;
                }
            }

            thing_resolve_collision(this, closest);

            set_remove(collided, closest);
        }

        set_delete(collided);

        for (int r = r_min; r <= r_max; r++) {
            for (int c = c_min; c <= c_max; c++) {
                Cell *current_cell = &this->map->cells[c + r * this->map->columns];
                for (int i = 0; i < current_cell->line_count; i++)
                    thing_line_collision(this, current_cell->lines[i]);
            }
        }

        thing_add_to_cells(this);
    }

    if (this->ground == false or FLOAT_NOT_ZERO(this->dy)) {

        this->dy -= gravity;
        this->y += this->dy;

        if (this->y < this->sec->floor) {
            this->ground = true;
            this->dy = 0;
            this->y = this->sec->floor;
        } else {
            this->ground = false;
        }
    }
}

void thing_initialize(Thing *this, World *map, float x, float z, float r, float box, float height) {

    this->id = thing_unique_id++;
    this->map = map;
    this->sec = world_find_sector(map, x, z);

    this->x = x;
    this->y = this->sec->floor;
    this->z = z;
    this->rotation = r;
    this->rotation_target = r;
    this->ground = true;

    this->box = box;
    this->height = height;

    thing_add_to_cells(this);

    world_add_thing(map, this);
}
