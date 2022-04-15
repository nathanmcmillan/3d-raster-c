/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "game.h"

enum Mode MODE = MODE_GAME;

Hymn *VM = NULL;

Input INPUT = {0};

Thing *PLAYER = NULL;

static HymnValue GetCameraHymn(Hymn *hymn, int count, HymnValue *arguments) {
    (void)hymn;
    (void)count;
    (void)arguments;
    HymnTable *table = hymn_new_table();
    return hymn_new_table_value(table);
}

void GameInit() {
    VIEW.radius = 8.0f;
    WorldInit();
    FontInit();
    SectorsInit();
    DrawInit();

    hymn_add_function(VM, "GetCamera", GetCameraHymn);
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

    // ...

    String *font_string = Read("pack/pictures/tic_80_wide_font.wad");
    Image *font_image = ImageRead(font_string);
    ResourceAddImage(font_image);
    StringFree(font_string);

    String *tiles_string = Read("pack/pictures/tiles.wad");
    Image *tiles_image = ImageRead(tiles_string);
    ResourceAddImage(tiles_image);
    StringFree(tiles_string);

    String *baron_string = Read("pack/pictures/baron.wad");
    Image *baron_image = ImageRead(baron_string);
    ResourceAddImage(baron_image);
    StringFree(baron_string);

    Image *font = ResourceImageSearch("TIC80WIDE");
    hymn_add_pointer(VM, "tic", font);

    // ...

    WorldClear();

    Wad *wad = WadParse(content);

    Array *map_vecs = WadGetArrayFromTable(wad, "vertex");
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
        WorldAddSector(sector_object);
    }

    WorldBuild(lines, line_count);

    Array *map_things = WadGetArrayFromTable(wad, "thing");
    if (map_things != NULL) {
        for (int i = 0; i < ArraySize(map_things); i++) {
            Wad *thing_wad = (Wad *)(map_things->items[i]);
            String *id = WadGetStringFromTable(thing_wad, "id");
            float x = WadGetFloatFromTable(thing_wad, "x");
            float z = WadGetFloatFromTable(thing_wad, "z");
            Thing *thing = WorldSpawnEntity(id, x, 0, z);
            if (strcmp(id, "player") == 0) {
                VIEW.x = x;
                VIEW.z = z;
                Sector *sector = WorldFindSector(x, z);
                if (sector == NULL) {
                    fprintf(stderr, "PLAYER NOT IN SECTOR <%g, %g>\n", x, z);
                    exit(1);
                }
                VIEW.sector = sector;
                VIEW.target = thing;
            }
        }
    }

    if (VIEW.sector == NULL) {
        fprintf(stderr, "PLAYER SECTOR NOT SET\n");
        exit(1);
    }

    Free(vecs);
    WadFree(wad);

    printf("done game open...\n");
}

static void GameUpdate() {
    if (INPUT.look_left) {
        VIEW.angle -= 0.1f;
    }

    if (INPUT.look_right) {
        VIEW.angle += 0.1f;
    }

    if (INPUT.look_up) {
        VIEW.look -= 0.1f;
    }

    if (INPUT.look_down) {
        VIEW.look += 0.1f;
    }

    if (INPUT.move_forward) {
        VIEW.x += cosf(VIEW.angle) * 0.1f;
        VIEW.z += sinf(VIEW.angle) * 0.1f;
    }

    if (INPUT.move_backward) {
        VIEW.x -= cosf(VIEW.angle) * 0.1f;
        VIEW.z -= sinf(VIEW.angle) * 0.1f;
    }

    if (INPUT.move_left) {
        VIEW.x += sinf(VIEW.angle) * 0.1f;
        VIEW.z -= cosf(VIEW.angle) * 0.1f;
    }

    if (INPUT.move_right) {
        VIEW.x -= sinf(VIEW.angle) * 0.1f;
        VIEW.z += cosf(VIEW.angle) * 0.1f;
    }

    if (INPUT.move_up) {
        VIEW.y += 0.1f;
    }

    if (INPUT.move_down) {
        VIEW.y -= 0.1f;
    }

    WorldUpdate();

    hymn_call(VM, "update", 0);
}

static void GameDraw() {
    CanvasClear();
    DrawWorld();
    // hymn_call(VM, "draw", 0);

    Image *font = ResourceImageSearch("TIC80WIDE");
    char *location = CharsFormat("x: %g, z: %g", VIEW.x, VIEW.z);
    CanvasText(font, 15, 15, location);
    Free(location);
}

void GameTick() {
    GameUpdate();
    GameDraw();
}

void GameFree() {
    SectorsFree();
    WorldFree();
}
