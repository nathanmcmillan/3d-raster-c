/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "game.h"

Game *new_game(Canvas *canvas, Hymn *vm, Input *input, Resources *resources) {
    Game *this = safe_calloc(1, sizeof(Game));
    this->state.canvas = canvas;
    this->state.vm = vm;
    this->state.input = input;
    this->state.resources = resources;
    this->state.update = game_update;
    this->state.draw = game_draw;
    this->world = new_world();
    this->camera = new_camera(8.0);
    u32 *palette = safe_calloc(256, sizeof(u32));
    palette[0] = 0xffffff;
    palette[1] = 0xff0000;
    palette[2] = 0x00ff00;
    palette[3] = 0x0000ff;
    this->palette = palette;
    return this;
}

static int texture(Resources *resources, String *name) {
    if (strcmp(name, "none")) {
        return -1;
    }
    return resources_image_name_to_index(resources, name);
}

void game_open(Game *this, String *content) {
    World *world = this->world;
    Resources *resources = this->state.resources;

    // ...

    // String *tiles_string = cat("pack/paint/tiles.wad");
    // ImageFile *tiles = read_image_file(tiles_string);
    // string_delete(tiles_string);
    // resources_add_image(resources, tiles->name, tiles->image);
    // image_file_delete(tiles); // fix table, need to copy string...

    // ...

    world_clear(world);

    MaybeWad parse = wad_parse(content);
    if (parse.error) {
        fprintf(stderr, parse.error);
        exit(1);
    }
    Wad *wad = parse.wad;

    WadArray *map_vecs = wad_get_array_from_object(wad, "vector");
    WadArray *map_lines = wad_get_array_from_object(wad, "line");
    WadArray *map_sectors = wad_get_array_from_object(wad, "sector");

    Array *vecs = new_array(0);
    Array *lines = new_array(0);

    for (usize i = 0; i < map_vecs->length; i++) {
        Wad *point = ((Wad *)map_vecs->items[i]);
        float x = wad_get_float(wad_get_from_object(point, "x"));
        float z = wad_get_float(wad_get_from_object(point, "z"));
        array_push(vecs, new_vec(x, z));
    }

    int line_id = 0;
    for (usize i = 0; i < map_lines->length; i++) {
        Wad *line = ((Wad *)map_lines->items[i]);
        int s = wad_get_int(wad_get_from_object(line, "s"));
        int e = wad_get_int(wad_get_from_object(line, "e"));
        Vec *a = array_get(vecs, s);
        Vec *b = array_get(vecs, e);
        array_push(lines, new_line(line_id, a, b, LINE_NO_SIDE, LINE_NO_SIDE, LINE_NO_SIDE));
        line_id++;
    }

    int sector_id = 0;
    for (usize i = 0; i < map_sectors->length; i++) {
        Wad *sector = ((Wad *)map_sectors->items[i]);
        float floor = wad_get_float(wad_get_from_object(sector, "f"));
        float ceiling = wad_get_float(wad_get_from_object(sector, "c"));
        int floor_image = texture(resources, wad_get_string(wad_get_from_object(sector, "b")));
        int ceiling_image = texture(resources, wad_get_string(wad_get_from_object(sector, "t")));
        WadArray *line_pointers = wad_get_array(wad_get_from_object(sector, "w"));
        int line_count = (int)array_size(line_pointers);
        Line **sector_lines = safe_calloc(line_count, sizeof(Line *));
        for (int d = 0; d < line_count; d++) {
            sector_lines[d] = array_get(lines, wad_get_int((Wad *)line_pointers->items[d]));
        }
        Sector *sector_object = new_sector(sector_id, sector_lines, line_count, floor, ceiling, floor_image, ceiling_image);
        world_add_sector(world, sector_object);
        sector_id++;
    }

    world_build(world, lines);

    WadArray *map_things = wad_get_array_from_object(wad, "thing");
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

    if (input->look_left) {
        camera->angle -= 0.1f;
    }

    if (input->look_right) {
        camera->angle += 0.1f;
    }

    if (input->look_up) {
        camera->look -= 0.1f;
    }

    if (input->look_down) {
        camera->look += 0.1f;
    }

    if (input->move_forward) {
        camera->x += cosf(camera->angle) * 0.1f;
        camera->z += sinf(camera->angle) * 0.1f;
    }

    if (input->move_backward) {
        camera->x -= cosf(camera->angle) * 0.1f;
        camera->z -= sinf(camera->angle) * 0.1f;
    }

    if (input->move_left) {
        camera->x += sinf(camera->angle) * 0.1f;
        camera->z -= cosf(camera->angle) * 0.1f;
    }

    if (input->move_right) {
        camera->x -= sinf(camera->angle) * 0.1f;
        camera->z += cosf(camera->angle) * 0.1f;
    }

    if (input->move_up) {
        camera->y += 0.1f;
    }

    if (input->move_down) {
        camera->y -= 0.1f;
    }

    hymn_call(this->state.vm, "update", 0);
}

void game_draw(void *state) {
    Game *this = (Game *)state;
    draw_world(this);
    hymn_call(this->state.vm, "draw", 0);
}

void game_delete(Game *this) {
    free(this);
}
