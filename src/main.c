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
#include "paint.h"
#include "resources.h"
#include "state.h"
#include "super.h"
#include "wad.h"

typedef struct Window Window;
typedef struct Main Main;

struct Window {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
};

struct Main {
    Window *win;
    Hymn *vm;
    Input input;
    Game *game;
    Paint *paint;
    State *state;
};

static bool run = true;

static void WindowInit(SDL_Window **win, SDL_Renderer **ren) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    u32 window_flags = SDL_WINDOW_SHOWN;
    SDL_Window *window = SDL_CreateWindow("Scroll and Sigil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    if (window == NULL) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }

    *win = window;

    u32 renderer_flags = SDL_RENDERER_TARGETTEXTURE;
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, renderer_flags);

    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        exit(1);
    }

    *ren = renderer;
}

static void sleeping(u32 time) {
    u32 diff = (1000 / 60) - (time - SDL_GetTicks());
    if (diff > 0) {
        SDL_Delay((diff < 15) ? diff : 15);
    }
}

static void poll_events(Input *in) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT: run = false; break;
        case SDL_MOUSEBUTTONUP: {
            SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
            in->mouse_down = false;
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
            in->mouse_down = true;
            break;
        }
        case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
            case SDLK_w: in->move_forward = false; break;
            case SDLK_a: in->move_left = false; break;
            case SDLK_s: in->move_backward = false; break;
            case SDLK_d: in->move_right = false; break;
            case SDLK_q: in->move_up = false; break;
            case SDLK_e: in->move_down = false; break;
            case SDLK_UP: in->look_up = false; break;
            case SDLK_DOWN: in->look_down = false; break;
            case SDLK_LEFT: in->look_left = false; break;
            case SDLK_RIGHT: in->look_right = false; break;
            }
            break;
        }
        case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: run = false; break;
            case SDLK_w: in->move_forward = true; break;
            case SDLK_a: in->move_left = true; break;
            case SDLK_s: in->move_backward = true; break;
            case SDLK_d: in->move_right = true; break;
            case SDLK_q: in->move_up = true; break;
            case SDLK_e: in->move_down = true; break;
            case SDLK_UP: in->look_up = true; break;
            case SDLK_DOWN: in->look_down = true; break;
            case SDLK_LEFT: in->look_left = true; break;
            case SDLK_RIGHT: in->look_right = true; break;
            case SDLK_TAB: in->console = true; break;
            }
            break;
        }
        }
    }
}

static void MainLoad(Main *game) {
    // String *map = Read("pack/maps/base.wad");
    String *map = Read("pack/maps/home.wad");
    game_open(game->game, map);
    string_delete(map);

    char *error = hymn_call(game->vm, "load", 0);
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }
}

static void WindowUpdate(Window *win) {
    SDL_UpdateTexture(win->texture, NULL, PIXELS, SCREEN_WIDTH * sizeof(u32));
    SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
}

static void MainLoop(Main *game) {
    Window *win = game->win;
    SDL_Renderer *renderer = win->renderer;

    u32 time = SDL_GetTicks();

    while (run) {
        poll_events(&game->input);

        game->state->update(game->state);
        game->state->draw(game->state);

        WindowUpdate(win);

        sleeping(time);
        SDL_RenderPresent(renderer);

        time = SDL_GetTicks();
    }
}

static void MainFree(Main *game) {
    hymn_delete(game->vm);
    CanvasFree();
    Free(game->win);
    Free(game);
}

static void switch_state(Main *game, void *state) {
    game->state = (State *)state;
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

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    CanvasInit(640, 400);

    WindowInit(&window, &renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    u32 pixel_format = SDL_PIXELFORMAT_ARGB8888;
    SDL_Texture *texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    Window *win = Calloc(1, sizeof(Window));
    win->window = window;
    win->renderer = renderer;
    win->texture = texture;

    Hymn *vm = new_hymn();

    // hymn_import_path("src\\<path>.hm");

    char *error = hymn_read(vm, "src/main.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    hymn_add_function(vm, "read_file", read_file);
    hymn_add_function(vm, "graphics_rect", CanvasRectangleHymn);

    Main *game = Calloc(sizeof(Main), 1);
    game->win = win;
    game->vm = vm;
    game->game = new_game(game->vm, &game->input);
    game->paint = new_paint(&game->input);
    switch_state(game, game->game);

    MainLoad(game);

    SDL_StartTextInput();

    MainLoop(game);

    SDL_StopTextInput();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    MainFree(game);
    ResourceFree();

    printf("exiting...\n");

    return 0;
}
