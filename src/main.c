/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "canvas.h"
#include "file_io.h"
#include "game.h"
#include "hymn.h"
#include "image.h"
#include "input.h"
#include "log.h"
#include "resources.h"
#include "super.h"
#include "wad.h"

static SDL_Window *WINDOW = NULL;
static SDL_Renderer *RENDERER = NULL;
static SDL_Texture *WINDOW_TEXTURE = NULL;

static bool RUN = true;

static void WindowInit() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    u32 window_flags = SDL_WINDOW_SHOWN;
    WINDOW = SDL_CreateWindow("Scroll and Sigil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    if (WINDOW == NULL) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }

    u32 renderer_flags = SDL_RENDERER_TARGETTEXTURE;
    RENDERER = SDL_CreateRenderer(WINDOW, -1, renderer_flags);

    if (RENDERER == NULL) {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        exit(1);
    }
}

static void sleeping(u32 time) {
    u32 diff = (1000 / 60) - (time - SDL_GetTicks());
    if (diff > 0) {
        SDL_Delay((diff < 15) ? diff : 15);
    }
}

static void PollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT: RUN = false; break;
        case SDL_MOUSEBUTTONUP: {
            SDL_GetMouseState(&INPUT.mouse_x, &INPUT.mouse_y);
            INPUT.mouse_down = false;
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            SDL_GetMouseState(&INPUT.mouse_x, &INPUT.mouse_y);
            INPUT.mouse_down = true;
            break;
        }
        case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
            case SDLK_w: INPUT.move_forward = false; break;
            case SDLK_a: INPUT.move_left = false; break;
            case SDLK_s: INPUT.move_backward = false; break;
            case SDLK_d: INPUT.move_right = false; break;
            case SDLK_q: INPUT.move_up = false; break;
            case SDLK_e: INPUT.move_down = false; break;
            case SDLK_UP: INPUT.look_up = false; break;
            case SDLK_DOWN: INPUT.look_down = false; break;
            case SDLK_LEFT: INPUT.look_left = false; break;
            case SDLK_RIGHT: INPUT.look_right = false; break;
            }
            break;
        }
        case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: RUN = false; break;
            case SDLK_w: INPUT.move_forward = true; break;
            case SDLK_a: INPUT.move_left = true; break;
            case SDLK_s: INPUT.move_backward = true; break;
            case SDLK_d: INPUT.move_right = true; break;
            case SDLK_q: INPUT.move_up = true; break;
            case SDLK_e: INPUT.move_down = true; break;
            case SDLK_UP: INPUT.look_up = true; break;
            case SDLK_DOWN: INPUT.look_down = true; break;
            case SDLK_LEFT: INPUT.look_left = true; break;
            case SDLK_RIGHT: INPUT.look_right = true; break;
            case SDLK_TAB: INPUT.console = true; break;
            }
            break;
        }
        }
    }
}

static void MainLoad() {
    String *map = Read("pack/maps/base.wad");
    GameOpen(map);
    StringFree(map);

    char *error = hymn_call(VM, "load", 0);
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }
}

static void WindowUpdate() {
    SDL_UpdateTexture(WINDOW_TEXTURE, NULL, PIXELS, SCREEN_WIDTH * sizeof(u32));
    SDL_RenderCopy(RENDERER, WINDOW_TEXTURE, NULL, NULL);
}

static void MainLoop() {
    u32 time = SDL_GetTicks();

    while (RUN) {
        PollEvents();

        switch (MODE) {
        case MODE_GAME:
            GameTick();
            break;
        case MODE_PAINT:
            PaintTick();
            break;
        }

        WindowUpdate();

        sleeping(time);
        SDL_RenderPresent(RENDERER);

        time = SDL_GetTicks();
    }
}

static HymnValue null_pointer(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    (void)count;
    (void)arguments;
    return hymn_new_pointer(NULL);
}

static HymnValue read_file(Hymn *this, int count, HymnValue *arguments) {
    (void)this;
    if (count != 1) {
        return hymn_new_none();
    }
    HymnString *string = hymn_as_string(arguments[0]);
    HymnString *content = Read(string);
    HymnObjectString *object = hymn_new_string_object(content);
    return hymn_new_string_value(object);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    CanvasInit(640, 400);

    WindowInit();

    SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
    SDL_RenderClear(RENDERER);
    SDL_RenderPresent(RENDERER);

    u32 pixel_format = SDL_PIXELFORMAT_ARGB8888;
    WINDOW_TEXTURE = SDL_CreateTexture(RENDERER, pixel_format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    VM = new_hymn();

    // hymn_import_path("src\\<path>.hm");

    char *error = hymn_read(VM, "src/main.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    hymn_add_function(VM, "read_file", read_file);
    hymn_add_function(VM, "canvas_rectangle", CanvasRectangleHymn);
    hymn_add_function(VM, "canvas_text", CanvasTextHymn);

    GameInit();
    PaintInit();

    MainLoad();

    SDL_StartTextInput();

    MainLoop();

    SDL_StopTextInput();
    SDL_DestroyTexture(WINDOW_TEXTURE);
    SDL_DestroyRenderer(RENDERER);
    SDL_DestroyWindow(WINDOW);
    SDL_Quit();

    hymn_delete(VM);

    CanvasFree();
    GameFree();
    PaintFree();
    ResourceFree();

    printf("exiting...\n");

    return 0;
}
