/* game Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with game
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "draw.h"

typedef struct Occluder Occluder;
typedef struct Plane Plane;
typedef struct VisibleSprite VisibleSprite;

struct Occluder {
    i32 top;
    i32 bottom;
    // Can we keep a depth here
    // So the whole single column of the screen has one depth
    // For use with clipping sprites
};

struct Plane {
    Line *line;
    float x1;
    float x2;
    float z1;
    float z2;
    Plane *next;
    float closest;
    Sector *sector;
};

struct VisibleSprite {
    i32 x1;
    i32 x2;
    float scale;
    float y;
    Image *sprite;
    VisibleSprite *next;
};

static const bool FIRST_PERSON = true;
static const bool DRAW_THINGS = false;

Occluder *OCCLUSION = NULL;

#define LINE_LIMIT 256
#define THING_LIMIT 256
#define SECTOR_LIMIT 256

Plane LINES[LINE_LIMIT];
Thing *THINGS[THING_LIMIT];
VisibleSprite SPRITES[THING_LIMIT];
Sector *SECTORS[SECTOR_LIMIT];

const float NEAR_Z = 1e-4f;
const float FAR_Z = 5.0f;
const float NEAR_SIDE = 1e-5f;
const float FAR_SIDE = 20.0f;

void DrawInit() {
    OCCLUSION = Malloc(SCREEN_WIDTH * sizeof(Occluder));
}

bool minf(float a, float b) {
    return (a < b) ? a : b;
}

bool maxf(float a, float b) {
    return (a > b) ? a : b;
}

bool clampf(float a, float lower, float upper) {
    return minf(minf(a, lower), upper);
}

bool overlap(float a0, float a1, float b0, float b1) {
    return (minf(a0, a1) <= maxf(b0, b1)) && (minf(b0, b1) <= maxf(a0, a1));
}

float cross(float x0, float y0, float x1, float y1) {
    return x0 * y1 - x1 * y0;
}

Vec intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    return (Vec){
        cross(cross(x1, y1, x2, y2), x1 - x2, cross(x3, y3, x4, y4), x3 - x4) / cross(x1 - x2, y1 - y2, x3 - x4, y3 - y4),
        cross(cross(x1, y1, x2, y2), y1 - y2, cross(x3, y3, x4, y4), y3 - y4) / cross(x1 - x2, y1 - y2, x3 - x4, y3 - y4),
    };
}

typedef struct Scaler Scaler;

struct Scaler {
    i32 result;
    i32 bop;
    i32 fd;
    i32 ca;
    i32 cache;
};

static Scaler ScalerInit(i32 a, i32 b, i32 c, i32 d, i32 f) {
    return (Scaler){
        d + (b - 1 - a) * (f - d) / (c - a),
        ((f < d) ^ (c < a)) ? -1 : 1,
        abs(f - d),
        abs(c - a),
        (i32)((b - 1 - a) * abs(f - d)) % abs(c - a),
    };
}

static i32 ScalerNext(Scaler *i) {
    for (i->cache += i->fd; i->cache >= i->ca; i->cache -= i->ca) {
        i->result += i->bop;
    }
    return i->result;
}

static void DrawColumn(i32 x, i32 y1, i32 y2, Image *image, Scaler scaler, i32 slice) {
    y1 = clamp32(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp32(y2, 0, SCREEN_HEIGHT - 1);
    i32 count = y2 - y1;
    if (count <= 0)
        return;
    u8 *source = image->pixels;
    u32 *destination = PIXELS + x + y1 * SCREEN_WIDTH;

    if (slice < 0)
        return;
    slice %= image->width;

    do {
        i32 texture_y = ScalerNext(&scaler);
        if (texture_y < 0)
            return;
        u8 color = source[slice + (texture_y % image->height) * image->width];
        if (color != UINT8_MAX)
            *destination = PALETTE[color];
        destination += SCREEN_WIDTH;
    } while (count-- != 0);

    // start = left starting X of wall
    // rw_scale = ScaleFromGlobalAngle(viewangle + xtoviewangle[start])

    // ds_p.scale2 =  ScaleFromGlobalAngle(viewangle + xtoviewangle[stop])
    // rw_scalestep = ds_p.scale2 - rw_scale / (stop - start)
    // ... rw_scale += rw_scalestep

    // dc_iscale =  0xffffffffu / rw_scale
    // fracstep = dc_iscale
    // centery -> screen_height / 2
    // vtop = floor_height + texture_height
    // dc_texturemid = vtop - viewz + sidedef.rowoffset
    // texturecolumn = rw_offset - finetangent[angle] * rw_distance
}

void DrawWorld() {
    float screen_center_width = (float)SCREEN_WIDTH / 2.0f;
    float screen_center_height = (float)SCREEN_HEIGHT / 2.0f;

    Image *tile = ResourceImageSearch("STONEFLOOR");

    float view_x = VIEW.x;
    float view_y = VIEW.y;
    float view_z = VIEW.z;

    float view_look = VIEW.look;

    float view_sin = sinf(VIEW.angle);
    float view_cos = cosf(VIEW.angle);

    Sector *sector = SectorSearchFor(VIEW.sector, view_x, view_z);
    if (sector != NULL)
        VIEW.sector = sector;

    float horizontal_fov = 0.73f * (float)SCREEN_WIDTH;
    float vertical_fov = 0.2f * (float)SCREEN_WIDTH;

    for (i32 i = 0; i < SCREEN_WIDTH; i++)
        OCCLUSION[i] = (Occluder){.top = -1, .bottom = SCREEN_HEIGHT};

    u32 tick = WORLD->tick;

    SECTORS[0] = sector;
    SECTORS[0]->visited = tick;

    int head = 1;
    int tail = 0;

    int lines_to_draw = 0;
    int things_to_draw = 0;

    while (true) {
        Sector *sector = SECTORS[tail];

        if (++tail == SECTOR_LIMIT)
            tail = 0;

        for (int w = 0; w < sector->line_count; w++) {
            if (lines_to_draw == LINE_LIMIT)
                break;
            Line *line = sector->lines[w];
            if (line->visited == tick)
                continue;
            line->visited = tick;
            Vec *a = line->a;
            Vec *b = line->b;
            // transform to camera view
            float transform_x_a = a->x - view_x;
            float transform_z_a = a->y - view_z;
            float transform_x_b = b->x - view_x;
            float transform_z_b = b->y - view_z;
            // rotate around camera view
            float space_z1 = transform_x_a * view_cos + transform_z_a * view_sin;
            float space_z2 = transform_x_b * view_cos + transform_z_b * view_sin;
            // is partially in front of camera
            if (space_z1 <= 0 && space_z2 <= 0)
                continue;
            float closest = space_z2 > space_z1 ? space_z2 : space_z1;
            float space_x1 = transform_x_a * view_sin - transform_z_a * view_cos;
            float space_x2 = transform_x_b * view_sin - transform_z_b * view_cos;
            LINES[lines_to_draw] = (Plane){.line = line, .x1 = space_x1, .x2 = space_x2, .z1 = space_z1, .z2 = space_z2, .closest = closest, .sector = sector};
            lines_to_draw++;
            // neighbor
            Sector *neighbor = NULL;
            if (line->front != NULL && line->front != sector) {
                // if (line->side_front.middle != LINE_NO_SIDE)
                // continue;
                neighbor = line->front;
            } else if (line->back != NULL && line->back != sector) {
                // if (line->side_back.middle != LINE_NO_SIDE)
                // continue;
                neighbor = line->back;
            } else {
                continue;
            }
            if (neighbor->visited == tick)
                continue;
            neighbor->visited = tick;
            SECTORS[head] = neighbor;
            if (++head == SECTOR_LIMIT)
                head = 0;
        }

        // need to be connected to a camera facing line to be worth processing ...
        for (int i = 0; i < sector->inside_count; i++) {
            Sector *inside = sector->inside[i];
            if (inside->visited == tick)
                continue;
            inside->visited = tick;
            SECTORS[head] = inside;
            if (++head == SECTOR_LIMIT)
                head = 0;
        }

        for (int t = 0; t < sector->thing_count; t++) {
            if (things_to_draw == THING_LIMIT)
                break;
            Thing *thing = sector->things[t];
            if (thing->visited == tick)
                continue;
            thing->visited = tick;
            if (FIRST_PERSON && thing == VIEW.target)
                continue;
            THINGS[things_to_draw] = thing;
            things_to_draw++;
        }

        if (tail == head) {
            break;
        }
    }

    Plane *plane = NULL;

    for (int p = 0; p < lines_to_draw; p++) {
        Plane *visit = &LINES[p];
        if (plane == NULL) {
            plane = visit;
            plane->next = NULL;
        } else {
            float closest = visit->closest;
            if (closest < plane->closest) {
                visit->next = plane;
                plane = visit;
            } else {
                Plane *previous = plane;
                Plane *current = plane->next;
                while (true) {
                    if (current == NULL) {
                        previous->next = visit;
                        visit->next = NULL;
                        break;
                    } else if (closest < current->closest) {
                        previous->next = visit;
                        visit->next = current;
                        break;
                    }
                    current = current->next;
                }
            }
        }
    }

    while (plane != NULL) {
        // Line *line = plane->line;
        float space_x1 = plane->x1;
        float space_x2 = plane->x2;
        float space_z1 = plane->z1;
        float space_z2 = plane->z2;
        // printf("%d | %g, %g | %g, %g\n", p, space_x1, space_x2, space_z1, space_z2);
        // Sector *sector;
        // if (space_x1 < space_x2) {
        //     sector = line->front;
        //     if (sector == NULL)
        //         sector = line->back;
        // } else {
        //     sector = line->back;
        //     if (sector == NULL)
        //         sector = line->front;
        //     float swap_x = space_x1;
        //     float swap_z = space_z1;
        //     space_x1 = space_x2;
        //     space_z1 = space_z2;
        //     space_x2 = swap_x;
        //     space_z2 = swap_z;
        // }
        // if (sector == NULL) {
        //     fprintf(stderr, "DRAW LINES NO SECTOR\n");
        //     exit(1);
        // }
        Sector *sector = plane->sector;

        i32 texture_modulo = tile->width - 1;
        i32 texture_u_0 = 0;
        i32 texture_u_1 = texture_modulo;
        // clip against camera view frustum
        if (space_z1 <= 0 || space_z2 <= 0) {
            float original_view_x_a = space_x1;
            float original_view_z_a = space_z1;
            float original_view_x_b = space_x2;
            float original_view_z_b = space_z2;
            Vec intersect_1 = intersect(space_x1, space_z1, space_x2, space_z2, -NEAR_SIDE, NEAR_Z, -FAR_SIDE, FAR_Z);
            Vec intersect_2 = intersect(space_x1, space_z1, space_x2, space_z2, NEAR_SIDE, NEAR_Z, FAR_SIDE, FAR_Z);
            if (space_z1 < NEAR_Z) {
                if (intersect_1.y > 0) {
                    space_x1 = intersect_1.x;
                    space_z1 = intersect_1.y;
                } else {
                    space_x1 = intersect_2.x;
                    space_z1 = intersect_2.y;
                }
            }
            if (space_z2 < NEAR_Z) {
                if (intersect_1.y > 0) {
                    space_x2 = intersect_1.x;
                    space_z2 = intersect_1.y;
                } else {
                    space_x2 = intersect_2.x;
                    space_z2 = intersect_2.y;
                }
            }
            if (fabsf(space_x2 - space_x1) > fabsf(space_z2 - space_z1)) {
                texture_u_0 = (space_x1 - original_view_x_a) * texture_modulo / (original_view_x_b - original_view_x_a);
                texture_u_1 = (space_x2 - original_view_x_a) * texture_modulo / (original_view_x_b - original_view_x_a);
            } else {
                texture_u_0 = (space_z1 - original_view_z_a) * texture_modulo / (original_view_z_b - original_view_z_a);
                texture_u_1 = (space_z2 - original_view_z_a) * texture_modulo / (original_view_z_b - original_view_z_a);
            }
        }
        // perspective transformation
        i32 x1 = (i32)(screen_center_width - space_x1 * (horizontal_fov / space_z1));
        i32 x2 = (i32)(screen_center_width - space_x2 * (horizontal_fov / space_z2));
        // is visible
        if (x1 >= x2 || x2 < 0 || x1 >= SCREEN_WIDTH)
            goto planes;
        float scale_y1 = vertical_fov / space_z1;
        float scale_y2 = vertical_fov / space_z2;
        float view_ceiling = sector->ceiling - view_y;
        float view_floor = sector->floor - view_y;
        // project floor and ceiling to screen coordinates
        i32 y1_ceiling = (i32)(screen_center_height - (view_ceiling + space_z1 * view_look) * scale_y1);
        i32 y2_ceiling = (i32)(screen_center_height - (view_ceiling + space_z2 * view_look) * scale_y2);
        i32 y1_floor = (i32)(screen_center_height - (view_floor + space_z1 * view_look) * scale_y1);
        i32 y2_floor = (i32)(screen_center_height - (view_floor + space_z2 * view_look) * scale_y2);
        // render
        i32 x = x1 < 0 ? 0 : x1;
        i32 end = x2;
        if (end >= SCREEN_WIDTH) end = SCREEN_WIDTH - 1;
        while (x <= end) {
            i32 ya = (x - x1) * (y2_ceiling - y1_ceiling) / (x2 - x1) + y1_ceiling;
            i32 cya = clamp32(ya, OCCLUSION[x].top, OCCLUSION[x].bottom);
            i32 yb = (x - x1) * (y2_floor - y1_floor) / (x2 - x1) + y1_floor;
            i32 cyb = clamp32(yb, OCCLUSION[x].top, OCCLUSION[x].bottom);
            Scaler scaler = ScalerInit(ya, cya, yb, 0, tile->width);
            i32 slice = (texture_u_0 * ((x2 - x) * space_z2) + texture_u_1 * ((x - x1) * space_z1)) / ((x2 - x) * space_z2 + (x - x1) * space_z1);
            DrawColumn(x, cya, cyb, tile, scaler, slice);

            // OCCLUSION[x].top =
            // OCCLUSION[x].bottom =

            x++;
        }

    planes:
        plane = plane->next;
    }

    if (things_to_draw == 0)
        return;

    if (!DRAW_THINGS)
        return;

    Image *sprite = ResourceImageSearch("BARONWALK0");

    VisibleSprite *visible = NULL;

    for (int t = 0; t < things_to_draw; t++) {
        Thing *thing = THINGS[t];
        float transform_x = thing->x - view_x;
        float transform_z = thing->z - view_z;
        float rotated_x = transform_x * view_cos;
        float rotated_z = -transform_z * view_sin;

        float space_z = rotated_x - rotated_z;

        if (space_z < NEAR_Z)
            continue;

        float scale = screen_center_width / space_z;

        rotated_x = -transform_x * view_sin;
        rotated_z = transform_z * view_cos;

        float space_x = -(rotated_z + rotated_x);

        if (fabs(space_x) > space_z * 2)
            continue;

        i32 sprite_width = sprite->width;

        i32 x1 = (i32)(screen_center_width + space_x * scale);

        if (x1 > SCREEN_WIDTH)
            continue;

        i32 x2 = (i32)(screen_center_width + (space_x + (float)sprite_width) * scale);

        if (x2 < 0)
            continue;

        if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;

        VisibleSprite *drawing = &SPRITES[t];

        drawing->x1 = x1;
        drawing->x2 = x2;
        drawing->scale = scale;
        drawing->y = thing->y;
        drawing->sprite = sprite;

        if (visible == NULL) {
            visible = drawing;
            visible->next = NULL;
        } else {
            if (scale < visible->scale) {
                drawing->next = visible;
                visible = drawing;
            } else {
                VisibleSprite *previous = visible;
                VisibleSprite *current = visible->next;
                while (true) {
                    if (current == NULL) {
                        previous->next = drawing;
                        drawing->next = NULL;
                        break;
                    } else if (scale < current->scale) {
                        previous->next = drawing;
                        drawing->next = current;
                        break;
                    }
                    current = current->next;
                }
            }
        }
    }

    while (visible != NULL) {
        i32 x1 = visible->x1;
        i32 x2 = visible->x2;
        float scale = visible->scale;
        Image *sprite = visible->sprite;
        u8 *source = sprite->pixels;
        i32 sprite_width = sprite->width;
        i32 sprite_height = sprite->height;

        float sprite_fraction = 0.0f;
        float inverse_scale = 1.0f / scale;

        if (x1 < 0) {
            x1 = 0;
            sprite_fraction = -x1 * scale;
        }

        float transform_y = visible->y + sprite_height - view_y; // + (space_x * view_look);

        i32 y1 = (i32)(screen_center_height - transform_y * scale);
        i32 y2 = (i32)((float)y1 + (float)sprite_height * scale);

        if (y1 < 0)
            y1 = 0;
        else if (y1 >= SCREEN_HEIGHT)
            y1 = SCREEN_HEIGHT - 1;

        if (y2 < 0)
            y2 = 0;
        else if (y2 >= SCREEN_HEIGHT)
            y2 = SCREEN_HEIGHT - 1;

        i32 count = y2 - y1;

        if (count <= 0)
            goto next;

        // float sprite_row_step = (float)(sprite_height - 1) / (float)count;

        // (ProjectSprite)
        // vis.gzt = thing.z + spritetopoffset[lump];
        // vis.texturemid = vis.gzt - rendererState.view.z;
        // vis.iscale = 1.0 / xscale

        // (DrawVisSprite)
        // maskedcvars.dc_iscale = vis.xiscale;
        // maskedcvars.dc_texturemid = vis.texturemid;
        // spryscale = vis.scale;
        // sprtopscreen = view.centeryfrac - maskedcvars.dc_texturemid * spryscale;

        // printf("DRAW SPRITE ... %g, %g | %d, %d | %d, %d | %d>\n", (transform_y + (y1 - screen_center_height) * inverse_scale), inverse_scale, x1, x2, y1, y2, sprite_height);

        for (i32 x = x1; x <= x2; x++, sprite_fraction += inverse_scale) {

            // (DrawMaskedColumn)
            // topscreen = sprtopscreen + spryscale * column.postdeltas[i];
            // bottomscreen = topscreen + spryscale * column.postlen[i];
            // dcvars = maskedcvars;

            // (DrawColumn)
            // fracstep = dcvars.dc_iscale;
            // frac = dcvars.dc_texturemid + (dcvars.dc_yl - dcvars.centery) * fracstep;

            i32 sprite_column = (i32)sprite_fraction;
            if (sprite_column < 0 || sprite_column >= sprite_width) {
                // fprintf(stderr, "DRAW THING BAD SPRITE COLUMN <%d, %g, %d, %d, %d>\n", sprite_column, inverse_scale, x1, x2, sprite_width);
                // exit(1);
                sprite_column = 0;
            }

            u32 *destination = PIXELS + x + y1 * SCREEN_WIDTH;

            float sprite_row_fraction = transform_y + (y1 - screen_center_height) * inverse_scale;

            do {
                i32 sprite_row = (i32)sprite_row_fraction;
                if (sprite_row < 0 || sprite_row >= sprite_height) {
                    // fprintf(stderr, "DRAW THING BAD SPRITE ROW <%d, %g, %d, %d, %d>\n", sprite_row, inverse_scale, y1, y2, sprite_height);
                    // exit(1);
                    sprite_row = 0;
                }
                u8 color = source[sprite_column + sprite_row * sprite_width];
                if (color != UINT8_MAX)
                    *destination = PALETTE[color];
                destination += SCREEN_WIDTH;
                sprite_row_fraction += inverse_scale;
            } while (count-- != 0);

            count = y2 - y1;
        }

    next:
        visible = visible->next;
    }
}

void DrawFree() {
    Free(OCCLUSION);
}
