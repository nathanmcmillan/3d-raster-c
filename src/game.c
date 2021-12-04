/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "game.h"

Game *new_game(Hymn *vm, Input *input) {
    Game *this = Calloc(1, sizeof(Game));
    this->state.vm = vm;
    this->state.input = input;
    this->state.update = game_update;
    this->state.draw = game_draw;
    this->world = new_world();
    this->camera = new_camera(8.0);
    return this;
}

static int texture(String *name) {
    if (strcmp(name, "none")) {
        return -1;
    }
    return ResourceImageIndex(name);
}

void game_open(Game *this, String *content) {
    printf("game open...\n");

    World *world = this->world;

    // ...

    // String *tiles_string = cat("pack/paint/tiles.wad");
    // ImageFile *tiles = read_image_file(tiles_string);
    // string_delete(tiles_string);
    // ResourceAddImage(resources, tiles->name, tiles->image);
    // image_file_delete(tiles); // fix table, need to copy string...

    String *font_string = Read("pack/paint/tic_80_wide_font.wad");
    Image *font_image = ImageRead(font_string);
    ResourceAddImage(font_image);
    // Wad *font_wad = wad_parse(font_string).wad;
    // wad_delete(font_wad);
    string_delete(font_string);

    // ...

    world_clear(world);

    MaybeWad parse = wad_parse(content);
    if (parse.error) {
        fprintf(stderr, "%s\n", parse.error);
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
        int floor_image = texture(wad_get_string(wad_get_from_object(sector, "b")));
        int ceiling_image = texture(wad_get_string(wad_get_from_object(sector, "t")));
        WadArray *line_pointers = wad_get_array(wad_get_from_object(sector, "w"));
        int line_count = (int)array_size(line_pointers);
        Line **sector_lines = Calloc(line_count, sizeof(Line *));
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

    printf("done game open...\n");
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
    Game *game = (Game *)state;
    draw_world(game);
    hymn_call(game->state.vm, "draw", 0);

    Image *font = ResourceImageSearch("TIC80WIDE");

    CanvasImage(font, 15, 15);
}

void game_delete(Game *this) {
    Free(this);
}
