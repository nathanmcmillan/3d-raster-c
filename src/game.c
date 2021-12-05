/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "game.h"

Hymn *VM = NULL;

Input INPUT = {0};

World *WORLD = NULL;
Camera *CAMERA = NULL;
Thing *PLAYER = NULL;

enum Mode MODE = MODE_GAME;

void GameInit() {
    WORLD = new_world();
    CAMERA = new_camera(8.0);
}

static int texture(char *name) {
    if (strcmp(name, "none") == 0) {
        return -1;
    }
    return ResourceImageIndex(name);
}

void GameOpen(String *content) {
    printf("game open...\n");

    World *world = WORLD;

    // ...

    String *font_string = Read("pack/paint/tic_80_wide_font.wad");
    Image *font_image = ImageRead(font_string);
    ResourceAddImage(font_image);
    StringFree(font_string);

    String *tiles_string = Read("pack/paint/tiles.wad");
    Image *tiles_image = ImageRead(tiles_string);
    ResourceAddImage(tiles_image);
    StringFree(tiles_string);

    // ...

    world_clear(world);

    MaybeWad parse = WadParse(content);
    if (parse.error) {
        fprintf(stderr, "%s\n", parse.error);
        exit(1);
    }
    Wad *wad = parse.wad;

    Array *map_vecs = WadGetArrayFromTable(wad, "vector");
    Array *map_lines = WadGetArrayFromTable(wad, "line");
    Array *map_sectors = WadGetArrayFromTable(wad, "sector");

    Array *vecs = new_array(0);
    Array *lines = new_array(0);

    for (usize i = 0; i < map_vecs->length; i++) {
        Wad *point = ((Wad *)map_vecs->items[i]);
        float x = WadGetFloatFromTable(point, "x");
        float z = WadGetFloatFromTable(point, "z");
        array_push(vecs, new_vec(x, z));
    }

    int line_id = 0;
    for (usize i = 0; i < map_lines->length; i++) {
        Wad *line = ((Wad *)map_lines->items[i]);
        int s = WadAsInt(WadGetFromTable(line, "s"));
        int e = WadAsInt(WadGetFromTable(line, "e"));
        Vec *a = ArrayGet(vecs, s);
        Vec *b = ArrayGet(vecs, e);
        array_push(lines, new_line(line_id, a, b, LINE_NO_SIDE, LINE_NO_SIDE, LINE_NO_SIDE));
        line_id++;
    }

    int sector_id = 0;
    for (usize i = 0; i < map_sectors->length; i++) {
        Wad *sector = ((Wad *)map_sectors->items[i]);
        float floor = WadGetFloatFromTable(sector, "f");
        float ceiling = WadGetFloatFromTable(sector, "c");
        int floor_image = texture(WadGetStringFromTable(sector, "b"));
        int ceiling_image = texture(WadGetStringFromTable(sector, "t"));
        Array *line_pointers = WadGetArrayFromTable(sector, "w");
        int line_count = (int)array_size(line_pointers);
        Line **sector_lines = Calloc(line_count, sizeof(Line *));
        for (int d = 0; d < line_count; d++) {
            sector_lines[d] = ArrayGet(lines, WadAsInt((Wad *)line_pointers->items[d]));
        }
        Sector *sector_object = new_sector(sector_id, sector_lines, line_count, floor, ceiling, floor_image, ceiling_image);
        world_add_sector(world, sector_object);
        sector_id++;
    }

    world_build(world, lines);

    Array *map_things = WadGetArrayFromTable(wad, "thing");
    if (map_things) {
        for (usize i = 0; i < map_things->length; i++) {
            Wad *thing = (Wad *)(map_things->items[i]);
            float x = WadGetFloatFromTable(thing, "x");
            float z = WadGetFloatFromTable(thing, "z");
            String *id = WadGetStringFromTable(thing, "id");

            if (strcmp(id, "hero")) {
                CAMERA->x = x;
                CAMERA->z = z;
                Sector *sector = world_find_sector(world, x, z);
                CAMERA->sector = sector != NULL ? sector : world->sectors[0];
            }
        }
    }

    array_delete(vecs);
    WadFree(wad);

    printf("done game open...\n");
}

static void GameUpdate() {
    Input *input = &INPUT;
    Camera *camera = CAMERA;

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

    hymn_call(VM, "update", 0);
}

static void GameDraw() {
    DrawWorld();
    hymn_call(VM, "draw", 0);

    // Image *font = ResourceImageSearch("TIC80WIDE");
    Image *font = ResourceImageSearch("TILES");

    CanvasImage(font, 15, 15);

    CanvasSprite(ImageSpriteSearch(font, "PLANKFLOOR"), 15, 300);
}

void GameTick() {
    GameUpdate();
    GameDraw();
}

void GameFree() {
}
