/* game Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with game
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "draw.h"

struct RenderY {
    i32 top;
    i32 bottom;
};

typedef struct RenderY RenderY;

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

#define INTERSECT_BOX(x0, y0, x1, y1, x2, y2, x3, y3) (OVERLAP(x0, x1, x2, x3) && OVERLAP(y0, y1, y2, y3))
#define POINT_SIDE(px, py, x0, y0, x1, y1) CROSS(x1 - x0, y1 - y0, px - x0, py - y0)

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

void draw_vertical_line(i32 x, i32 y1, i32 y2, u32 top, u32 middle, u32 bottom) {
    y1 = clamp32(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp32(y2, 0, SCREEN_HEIGHT - 1);
    if (y2 == y1) {
        PIXELS[y1 * SCREEN_WIDTH + x] = middle;
    } else if (y2 > y1) {
        PIXELS[y1 * SCREEN_WIDTH + x] = top;
        for (i32 y = y1 + 1; y < y2; y++) {
            PIXELS[y * SCREEN_WIDTH + x] = middle;
        }
        PIXELS[y2 * SCREEN_WIDTH + x] = bottom;
    }
}

struct Scaler {
    int result, bop, fd, ca, cache;
};

typedef struct Scaler Scaler;

Scaler new_scaler(int a, int b, int c, int d, int f) {
    return (Scaler){
        d + (b - 1 - a) * (f - d) / (c - a),
        ((f < d) ^ (c < a)) ? -1 : 1,
        abs(f - d),
        abs(c - a),
        (int)((b - 1 - a) * abs(f - d)) % abs(c - a),
    };
}

static int scaler_next(struct Scaler *i) {
    for (i->cache += i->fd; i->cache >= i->ca; i->cache -= i->ca) {
        i->result += i->bop;
    }
    return i->result;
}

void draw_vertical_line_image(i32 x, i32 y1, i32 y2, Scaler scaler, i32 txtx, Image *image, u32 *palette) {
    y1 = clamp32(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp32(y2, 0, SCREEN_HEIGHT - 1);
    u32 *pixels = PIXELS + (y1 * SCREEN_WIDTH + x);
    i32 image_width = image->width;
    for (i32 y = y1; y <= y2; y++) {
        i32 txty = scaler_next(&scaler);
        *pixels = palette[image->pixels[txtx % image_width + (txty % image_width) * image_width]];
        pixels += SCREEN_WIDTH;
    }
}

#define YAW(y, z) (y + z * camera_look)

void DrawWorld() {
    i32 screen_width_half = SCREEN_WIDTH / 2;
    i32 screen_height_half = SCREEN_HEIGHT / 2;

    World *world = WORLD;
    Camera *camera = CAMERA;

    CanvasClear();

    CanvasRectangle(rgb(255, 255, 0), 10, 60, 42, 92);

    // float perspective[16];
    // float view[16];
    // float projection[16];

    // float fov = 60.0f;
    // float ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    // float near = 0.01f;
    // float far = 100.0f;

    float camera_x = camera->x;
    float camera_y = camera->y;
    float camera_z = camera->z;

    float camera_look = camera->look;

    float camera_sin = sinf(camera->angle);
    float camera_cos = cosf(camera->angle);

    Sector *find = world_find_sector(world, camera_x, camera_z);
    if (find) camera->sector = find;

    float horizontal_fov = 0.73f * (float)SCREEN_WIDTH;
    float vertical_fov = 0.2f * (float)SCREEN_WIDTH;

    const float near_z = 1e-4f;
    const float far_z = 5.0f;
    const float near_side = 1e-5f;
    const float far_side = 20.0f;

    RenderY *remaining = Calloc(SCREEN_WIDTH, sizeof(RenderY));
    for (i32 i = 0; i < SCREEN_WIDTH; i++) {
        remaining[i].bottom = SCREEN_HEIGHT - 1;
    }

#define QUEUE_COUNT 32
    Visit queue[QUEUE_COUNT];

    u32 tick = world->tick;
    u32 *sectors_visited = world->sectors_visited;

    Visit *head = queue + 1;
    Visit *tail = queue;

    *tail = (Visit){camera->sector, 0, SCREEN_WIDTH - 1};
    while (true) {
        Visit current = *tail;
        Sector *sector = current.sector;

        if (++tail == queue + QUEUE_COUNT) {
            tail = queue;
        }

        sectors_visited[sector->id] = tick;

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
                Vec i1 = intersect(tx1, tz1, tx2, tz2, -near_side, near_z, -far_side, far_z);
                Vec i2 = intersect(tx1, tz1, tx2, tz2, near_side, near_z, far_side, far_z);
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
            float yscale1 = vertical_fov / tz1;
            i32 x1 = screen_width_half - (i32)(tx1 * xscale1);
            float xscale2 = horizontal_fov / tz2;
            float yscale2 = vertical_fov / tz2;
            i32 x2 = screen_width_half - (i32)(tx2 * xscale2);
            // is visible
            if (x1 >= x2 || x2 < current.sx1 || x1 > current.sx2) {
                continue;
            }
            float yceil = sector->ceiling - camera_y;
            float yfloor = sector->floor - camera_y;
            // neighbor
            Sector *neighbor = NULL;
            float nyceil = 0.0f;
            float nyfloor = 0.0f;
            // if (line->front != NULL && line->front != sector) {
            //     neighbor = line->front;
            // } else if (line->back != NULL && line->back != sector) {
            //     neighbor = line->back;
            // }
            if (neighbor != NULL) {
                nyceil = neighbor->ceiling - camera_y;
                nyfloor = neighbor->floor - camera_y;
            }
            // project floor and ceiling to screen coordinates
            i32 y1a = screen_height_half - (i32)(YAW(yceil, tz1) * yscale1);
            i32 y1b = screen_height_half - (i32)(YAW(yfloor, tz1) * yscale1);
            i32 y2a = screen_height_half - (i32)(YAW(yceil, tz2) * yscale2);
            i32 y2b = screen_height_half - (i32)(YAW(yfloor, tz2) * yscale2);
            i32 ny1a = screen_height_half - (i32)(YAW(nyceil, tz1) * yscale1);
            i32 ny1b = screen_height_half - (i32)(YAW(nyfloor, tz1) * yscale1);
            i32 ny2a = screen_height_half - (i32)(YAW(nyceil, tz2) * yscale2);
            i32 ny2b = screen_height_half - (i32)(YAW(nyfloor, tz2) * yscale2);
            // render
            i32 beginx = max32(x1, current.sx1);
            i32 endx = min32(x2, current.sx2);
            for (i32 x = beginx; x <= endx; x++) {
                i32 ya = (x - x1) * (y2a - y1a) / (x2 - x1) + y1a;
                i32 cya = clamp32(ya, remaining[x].top, remaining[x].bottom);
                i32 yb = (x - x1) * (y2b - y1b) / (x2 - x1) + y1b;
                i32 cyb = clamp32(yb, remaining[x].top, remaining[x].bottom);
                draw_vertical_line(x, remaining[x].top, cya - 1, 0x111111, 0x222222, 0x111111);
                draw_vertical_line(x, cyb + 1, remaining[x].bottom, 0x0000ff, 0x0000aa, 0x0000ff);
                if (neighbor != NULL) {
                    i32 nya = (x - x1) * (ny2a - ny1a) / (x2 - x1) + ny1a;
                    i32 cnya = clamp32(nya, remaining[x].top, remaining[x].bottom);
                    i32 nyb = (x - x1) * (ny2b - ny1b) / (x2 - x1) + ny1b;
                    i32 cnyb = clamp32(nyb, remaining[x].top, remaining[x].bottom);
                    remaining[x].top = clamp32(max32(cya, cnya), remaining[x].top, SCREEN_HEIGHT - 1);
                    remaining[x].bottom = clamp32(min32(cyb, cnyb), 0, remaining[x].bottom);
                    draw_vertical_line(x, cya, cnya - 1, 0, x == x1 || x == x2 ? 0 : 0x010101, 0);
                    draw_vertical_line(x, cnyb + 1, cyb, 0, x == x1 || x == x2 ? 0 : 0x040007, 0);
                } else {
                    draw_vertical_line(x, cya, cyb, 0, x == x1 || x == x2 ? 0 : 0xaaaaaa, 0);
                }
            }
            // schedule neighbor for rendering
            if (neighbor != NULL && endx >= beginx && (head + QUEUE_COUNT - tail) % QUEUE_COUNT) {
                if (sectors_visited[neighbor->id] != tick) {
                    *head = (Visit){neighbor, beginx, endx};
                    if (++head == queue + QUEUE_COUNT) head = queue;
                    printf("portal...\n");
                }
            }
        }

        if (tail == head) {
            break;
        }
    }
}
