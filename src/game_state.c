/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "state.h"

GameState *new_game_state(Canvas *canvas, Input *input, Assets *assets) {
    GameState *this = safe_calloc(1, sizeof(GameState));
    this->state.canvas = canvas;
    this->state.input = input;
    this->state.assets = assets;
    this->state.update = game_state_update;
    this->state.draw = game_state_draw;
    this->camera = new_camera(8.0);
    return this;
}

static int texture(Assets *assets, String *name) {
    if (strcmp(name, "none")) {
        return -1;
    }
    return assets_paint_name_to_index(assets, name);
}

void game_state_open(GameState *this, String *content) {
    World *world = this->world;
    Assets *assets = this->state.assets;

    world_clear(world);

    MaybeWad parse = wad_parse(content);
    if (parse.error) {
        fprintf(stderr, parse.error);
        exit(1);
    }
    Wad *wad = parse.wad;
    // printf("read: %s\n", wad_to_string(wad));
    // printf("map: %s\n", wad_get_string_from_object(wad, "map"));

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
        array_push(vecs, new_line(a, b, LINE_NO_WALL, LINE_NO_WALL, LINE_NO_WALL));
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
        world_add_sector(world, new_sector(sector_vecs, vec_count, sector_lines, line_count, bottom, floor, ceiling, top, floor_paint, ceiling_paint));
    }

    world_build(world, lines);

    array_delete(vecs);
    wad_delete(wad);
}

void game_state_update(void *state) {
    GameState *this = (GameState *)state;

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
}

void game_state_draw(void *state) {
    GameState *this = (GameState *)state;

    Canvas *canvas = this->state.canvas;
    Camera *camera = this->camera;

    canvas_rect(canvas, rgb(255, 255, 0), 10, 60, 42, 92);

    canvas_clear_depth(canvas);

    float perspective[16];
    float view[16];
    float projection[16];

    i32 width = this->state.canvas->width;
    i32 height = this->state.canvas->height;

    float fov = 60.0f;
    float ratio = (float)width / (float)height;
    float near = 0.01f;
    float far = 100.0f;

    matrix_perspective(perspective, fov, near, far, ratio);

    matrix_identity(view);
    matrix_rotate_x(view, sinf(camera->rx), cosf(camera->rx));
    matrix_rotate_y(view, sinf(camera->ry), cosf(camera->ry));
    matrix_translate(view, -camera->x, -camera->y, -camera->z);
    matrix_multiply(projection, perspective, view);

    i32 vertex_stride = 8;
    // i32 vertex_count = 8;
    float mesh[] = {
        -1,
        +1,
        +1,
        0,
        255,
        0,
        0,
        0,
        +1,
        +1,
        +1,
        255,
        255,
        0,
        1,
        0,
        -1,
        -1,
        +1,
        0,
        255,
        255,
        0,
        1,
        +1,
        -1,
        +1,
        255,
        0,
        0,
        1,
        1,
        -1,
        +1,
        -1,
        255,
        0,
        255,
        0,
        0,
        +1,
        +1,
        -1,
        0,
        0,
        255,
        1,
        0,
        +1,
        -1,
        -1,
        255,
        0,
        0,
        1,
        1,
        -1,
        -1,
        -1,
        0,
        255,
        0,
        0,
        1,
    };

    i32 index_count = 12;
    i32 indices[] = {
        0,
        1,
        2,
        1,
        2,
        3,
        1,
        3,
        6,
        1,
        5,
        6,
        0,
        1,
        4,
        1,
        4,
        5,
        2,
        3,
        7,
        3,
        6,
        7,
        0,
        2,
        7,
        0,
        4,
        7,
        4,
        5,
        6,
        4,
        6,
        7,
    };

    float oa[4];
    float ob[4];
    float oc[4];

    for (int i = 0; i < index_count; i += 3) {

        float *a = &mesh[indices[i] * vertex_stride];
        float *b = &mesh[indices[i + 1] * vertex_stride];
        float *c = &mesh[indices[i + 2] * vertex_stride];

        canvas_project(canvas, oa, projection, a);
        canvas_project(canvas, ob, projection, b);
        canvas_project(canvas, oc, projection, c);

        canvas_rasterize(canvas, oa, ob, oc);
    }
}

void game_state_delete(GameState *this) {
    free(this);
}
