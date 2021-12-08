/* game Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with game
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "draw.h"

typedef struct RenderX RenderX;
typedef struct RenderY RenderY;
typedef struct Visit Visit;

struct RenderX {
    i32 first;
    i32 last;
};

struct RenderY {
    i32 top;
    i32 bottom;
};

struct Visit {
    Sector *sector;
    i32 sx1;
    i32 sx2;
};

#define EYE_HEIGHT 6
#define DUCK_HEIGHT 2.5f
#define HEAD_MARGIN 1
#define KNEE_HEIGHT 2

#define INTERSECT_BOX(x0, y0, x_1, y1, x_2, y2, x3, y3) (OVERLAP(x0, x_1, x_2, x3) && OVERLAP(y0, y1, y2, y3))
#define POINT_SIDE(px, py, x0, y0, x_1, y1) CROSS(x_1 - x0, y1 - y0, px - x0, py - y0)

i32 SEGMENT_LIMIT = 0;
i32 SEGMENT_COUNT = 0;
RenderX *SEGMENT = NULL;
RenderY *CLIP = NULL;

void DrawInit() {
    SEGMENT_LIMIT = SCREEN_WIDTH / 2 + 1;
    SEGMENT = Malloc(SEGMENT_LIMIT, sizeof(RenderX));
    CLIP = Malloc(SCREEN_WIDTH, sizeof(RenderY));
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

float cross(float x0, float y0, float x_1, float y1) {
    return x0 * y1 - x_1 * y0;
}

Vec intersect(float x_1, float y1, float x_2, float y2, float x3, float y3, float x4, float y4) {
    return (Vec){
        cross(cross(x_1, y1, x_2, y2), x_1 - x_2, cross(x3, y3, x4, y4), x3 - x4) / cross(x_1 - x_2, y1 - y2, x3 - x4, y3 - y4),
        cross(cross(x_1, y1, x_2, y2), y1 - y2, cross(x3, y3, x4, y4), y3 - y4) / cross(x_1 - x_2, y1 - y2, x3 - x4, y3 - y4),
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

void draw_vertical_line_image(i32 x, i32 y1, i32 y2, Image *image, Scaler scaler, i32 slice) {
    y1 = clamp32(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp32(y2, 0, SCREEN_HEIGHT - 1);
    i32 count = y2 - y1;
    if (count <= 0)
        return;
    u8 *source = image->pixels;
    u32 *destination = PIXELS + x + y1 * SCREEN_WIDTH;

    // i32 image_bounds = image->width * image->height;
    // i32 step = image->width;
    // do {
    //     u8 d = source[slice % image_bounds];
    //     if (d != UINT8_MAX)
    //         *destination = PALETTE[d];
    //     destination += SCREEN_WIDTH;
    //     slice += step;
    // } while (count--);

    slice %= image->width;
    do {
        i32 texture_y = ScalerNext(&scaler);
        u8 d = source[slice + (texture_y % image->height) * image->width];
        if (d != UINT8_MAX)
            *destination = PALETTE[d];
        destination += SCREEN_WIDTH;
    } while (count--);

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

#define YAW(y, z) (y + z * camera_look)

void DrawWorld() {
    i32 screen_center_width = SCREEN_WIDTH / 2;
    i32 screen_center_height = SCREEN_HEIGHT / 2;

    World *world = WORLD;
    Camera *camera = CAMERA;

    Image *tile = ResourceImageSearch("STONEFLOOR");

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
    if (find != NULL)
        camera->sector = find;

    float horizontal_fov = 0.73f * (float)SCREEN_WIDTH;
    float vertical_fov = 0.2f * (float)SCREEN_WIDTH;

    const float near_z = 1e-4f;
    const float far_z = 5.0f;
    const float near_side = 1e-5f;
    const float far_side = 20.0f;

    SEGMENT[0].first = INT32_MIN;
    SEGMENT[1].last = -1;
    SEGMENT[0].first = SCREEN_WIDTH;
    SEGMENT[1].last = INT32_MAX;
    SEGMENT_COUNT = 2;

    for (i32 i = 0; i < SCREEN_WIDTH; i++)
        CLIP[i] = (RenderY){-1, SCREEN_HEIGHT};

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

        for (int w = 0; w < sector->line_count; w++) {
            Line *line = sector->lines[w];
            Vec *a = line->a;
            Vec *b = line->b;
            // transform to camera view
            float camera_space_x_a = a->x - camera_x;
            float camera_space_z_a = a->y - camera_z;
            float camera_space_x_b = b->x - camera_x;
            float camera_space_z_b = b->y - camera_z;
            // rotate around camera view
            float view_x_1 = camera_space_x_a * camera_sin - camera_space_z_a * camera_cos;
            float view_z_1 = camera_space_x_a * camera_cos + camera_space_z_a * camera_sin;
            float view_x_2 = camera_space_x_b * camera_sin - camera_space_z_b * camera_cos;
            float view_z_2 = camera_space_x_b * camera_cos + camera_space_z_b * camera_sin;
            // is partially in front of camera
            if (view_z_1 <= 0 && view_z_2 <= 0)
                continue;
            i32 texture_modulo = tile->width - 1;
            i32 texture_u_0 = 0;
            i32 texture_u_1 = texture_modulo;
            // clip against camera view frustum
            if (view_z_1 <= 0 || view_z_2 <= 0) {
                float original_view_x_a = view_x_1;
                float original_view_z_a = view_z_1;
                float original_view_x_b = view_x_2;
                float original_view_z_b = view_z_2;
                Vec intersect_1 = intersect(view_x_1, view_z_1, view_x_2, view_z_2, -near_side, near_z, -far_side, far_z);
                Vec intersect_2 = intersect(view_x_1, view_z_1, view_x_2, view_z_2, near_side, near_z, far_side, far_z);
                if (view_z_1 < near_z) {
                    if (intersect_1.y > 0) {
                        view_x_1 = intersect_1.x;
                        view_z_1 = intersect_1.y;
                    } else {
                        view_x_1 = intersect_2.x;
                        view_z_1 = intersect_2.y;
                    }
                }
                if (view_z_2 < near_z) {
                    if (intersect_1.y > 0) {
                        view_x_2 = intersect_1.x;
                        view_z_2 = intersect_1.y;
                    } else {
                        view_x_2 = intersect_2.x;
                        view_z_2 = intersect_2.y;
                    }
                }
                if (fabsf(view_x_2 - view_x_1) > fabsf(view_z_2 - view_z_1)) {
                    texture_u_0 = (view_x_1 - original_view_x_a) * texture_modulo / (original_view_x_b - original_view_x_a);
                    texture_u_1 = (view_x_2 - original_view_x_a) * texture_modulo / (original_view_x_b - original_view_x_a);
                } else {
                    texture_u_0 = (view_z_1 - original_view_z_a) * texture_modulo / (original_view_z_b - original_view_z_a);
                    texture_u_1 = (view_z_2 - original_view_z_a) * texture_modulo / (original_view_z_b - original_view_z_a);
                }
            }
            // perspective transformation
            i32 x_1 = screen_center_width - (i32)(view_x_1 * (horizontal_fov / view_z_1));
            i32 x_2 = screen_center_width - (i32)(view_x_2 * (horizontal_fov / view_z_2));
            // is visible
            if (x_1 >= x_2 || x_2 < current.sx1 || x_1 > current.sx2)
                continue;
            float scale_y_1 = vertical_fov / view_z_1;
            float scale_y_2 = vertical_fov / view_z_2;
            float view_ceiling = sector->ceiling - camera_y;
            float view_floor = sector->floor - camera_y;
            // neighbor
            // Sector *neighbor = NULL;
            // float nyceil = 0.0f;
            // float nyfloor = 0.0f;
            // if (line->front != NULL && line->front != sector) {
            //     neighbor = line->front;
            // } else if (line->back != NULL && line->back != sector) {
            //     neighbor = line->back;
            // }
            // if (neighbor != NULL) {
            //     nyceil = neighbor->ceiling - camera_y;
            //     nyfloor = neighbor->floor - camera_y;
            // }
            // project floor and ceiling to screen coordinates
            i32 y_1_ceiling = screen_center_height - (i32)(YAW(view_ceiling, view_z_1) * scale_y_1);
            i32 y_2_ceiling = screen_center_height - (i32)(YAW(view_ceiling, view_z_2) * scale_y_2);
            i32 y_1_floor = screen_center_height - (i32)(YAW(view_floor, view_z_1) * scale_y_1);
            i32 y_2_floor = screen_center_height - (i32)(YAW(view_floor, view_z_2) * scale_y_2);
            // i32 ny1a = screen_center_height - (i32)(YAW(nyceil, view_z_1) * scale_y_1);
            // i32 ny1b = screen_center_height - (i32)(YAW(nyfloor, view_z_1) * scale_y_1);
            // i32 ny2a = screen_center_height - (i32)(YAW(nyceil, view_z_2) * scale_y_2);
            // i32 ny2b = screen_center_height - (i32)(YAW(nyfloor, view_z_2) * scale_y_2);
            // render
            i32 begin_x = max32(x_1, current.sx1);
            i32 end_x = min32(x_2, current.sx2);
            for (i32 x = begin_x; x <= end_x; x++) {
                i32 ya = (x - x_1) * (y_2_ceiling - y_1_ceiling) / (x_2 - x_1) + y_1_ceiling;
                i32 cya = clamp32(ya, CLIP[x].top, CLIP[x].bottom);
                i32 yb = (x - x_1) * (y_2_floor - y_1_floor) / (x_2 - x_1) + y_1_floor;
                i32 cyb = clamp32(yb, CLIP[x].top, CLIP[x].bottom);
                draw_vertical_line(x, CLIP[x].top, cya - 1, 0x111111, 0x222222, 0x111111);
                draw_vertical_line(x, cyb + 1, CLIP[x].bottom, 0x0000ff, 0x0000aa, 0x0000ff);
                // if (neighbor != NULL) {
                //     i32 nya = (x - x_1) * (ny2a - ny1a) / (x_2 - x_1) + ny1a;
                //     i32 cnya = clamp32(nya, CLIP[x].top, CLIP[x].bottom);
                //     i32 nyb = (x - x_1) * (ny2b - ny1b) / (x_2 - x_1) + ny1b;
                //     i32 cnyb = clamp32(nyb, CLIP[x].top, CLIP[x].bottom);
                //     CLIP[x].top = clamp32(max32(cya, cnya), CLIP[x].top, SCREEN_HEIGHT - 1);
                //     CLIP[x].bottom = clamp32(min32(cyb, cnyb), 0, CLIP[x].bottom);
                //     draw_vertical_line(x, cya, cnya - 1, 0, x == x_1 || x == x_2 ? 0 : 0x010101, 0);
                //     draw_vertical_line(x, cnyb + 1, cyb, 0, x == x_1 || x == x_2 ? 0 : 0x040007, 0);
                // } else {
                // draw_vertical_line(x, cya, cyb, 0, x == x_1 || x == x_2 ? 0 : 0xaaaaaa, 0);
                Scaler scaler = ScalerInit(ya, cya, yb, 0, tile->width);
                i32 slice = (texture_u_0 * ((x_2 - x) * view_z_2) + texture_u_1 * ((x - x_1) * view_z_1)) / ((x_2 - x) * view_z_2 + (x - x_1) * view_z_1);
                draw_vertical_line_image(x, cya, cyb, tile, scaler, slice);
                // }
            }
            // schedule neighbor for rendering
            // if (neighbor != NULL && end_x >= begin_x && (head + QUEUE_COUNT - tail) % QUEUE_COUNT) {
            //     if (sectors_visited[neighbor->id] != tick) {
            //         *head = (Visit){neighbor, begin_x, end_x};
            //         if (++head == queue + QUEUE_COUNT) head = queue;
            //         printf("portal...\n");
            //     }
            // }
        }

        if (tail == head) {
            break;
        }
    }
}

void DrawFree() {
    Free(SEGMENT);
    Free(CLIP);
}
