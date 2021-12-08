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
    // Sprite ID as name in image index?
    // Special walls/floors/ceils sprite index
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

    WorldClear(world);

    MaybeWad parse = WadParse(content);
    if (parse.error) {
        fprintf(stderr, "%s\n", parse.error);
        exit(1);
    }
    Wad *wad = parse.wad;

    Array *map_vecs = WadGetArrayFromTable(wad, "vector");
    Array *map_lines = WadGetArrayFromTable(wad, "line");
    Array *map_sectors = WadGetArrayFromTable(wad, "sector");

    int vec_count = ArraySize(map_vecs);
    int line_count = ArraySize(map_lines);

    Vec **vecs = Malloc(vec_count * sizeof(Vec *));
    Line **lines = Malloc(line_count * sizeof(Line *));

    for (int i = 0; i < vec_count; i++) {
        Wad *point = ((Wad *)map_vecs->items[i]);
        float x = WadGetFloatFromTable(point, "x");
        float z = WadGetFloatFromTable(point, "z");
        vecs[i] = new_vec(x, z);
    }

    for (int i = 0; i < line_count; i++) {
        Wad *line = ((Wad *)map_lines->items[i]);
        int s = WadGetIntFromTable(line, "s");
        int e = WadGetIntFromTable(line, "e");
        Vec *a = vecs[s];
        Vec *b = vecs[e];
        Side front = {0};
        Side back = {0};
        Wad *p = WadGetFromTable(line, "f");
        Wad *m = WadGetFromTable(line, "b");
        if (p != NULL) {
            front.top = texture(WadGetStringFromTable(p, "t"));
            front.middle = texture(WadGetStringFromTable(p, "m"));
            front.bottom = texture(WadGetStringFromTable(p, "b"));
        }
        if (m != NULL) {
            back.top = texture(WadGetStringFromTable(m, "t"));
            back.middle = texture(WadGetStringFromTable(m, "m"));
            back.bottom = texture(WadGetStringFromTable(m, "b"));
        }
        lines[i] = NewLine(i, a, b, front, back);
    }

    for (int i = 0; i < ArraySize(map_sectors); i++) {
        Wad *sector = ((Wad *)map_sectors->items[i]);
        float floor = WadGetFloatFromTable(sector, "f");
        float ceiling = WadGetFloatFromTable(sector, "c");
        int floor_image = texture(WadGetStringFromTable(sector, "b"));
        int ceiling_image = texture(WadGetStringFromTable(sector, "t"));
        Array *line_pointers = WadGetArrayFromTable(sector, "w");
        int sector_line_count = ArraySize(line_pointers);
        Line **sector_lines = Calloc(sector_line_count, sizeof(Line *));
        for (int d = 0; d < sector_line_count; d++) {
            sector_lines[d] = lines[WadAsInt(line_pointers->items[d])];
        }
        Sector *sector_object = NewSector(i, sector_lines, sector_line_count, floor, ceiling, floor_image, ceiling_image);
        world_add_sector(world, sector_object);
    }

    WorldBuild(world, lines, line_count);

    Array *map_things = WadGetArrayFromTable(wad, "thing");
    if (map_things != NULL) {
        for (int i = 0; i < ArraySize(map_things); i++) {
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

    Free(vecs);
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

    Image *font = ResourceImageSearch("TIC80WIDE");
    CanvasImage(font, 15, 15);

    // Image *tiles = ResourceImageSearch("TILES");
    // CanvasImage(tiles, 115, 15);
}

void GameTick() {
    GameUpdate();
    GameDraw();
}

void GameFree() {
}
