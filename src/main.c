/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <SDL.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "canvas.h"
#include "file_io.h"
#include "hymn.h"
#include "input.h"
#include "log.h"
#include "paint.h"
#include "pie.h"
#include "state.h"
#include "wad.h"

typedef struct Window Window;
typedef struct Main Main;

struct Window {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Canvas *canvas;
};

struct Main {
    Window *win;
    Hymn *vm;
    Input input;
    Game *game;
    Paint *paint;
    State *state;
};

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 400;

static bool run = true;

static void window_init(SDL_Window **win, SDL_Renderer **ren) {

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

static void main_load(Main *game) {
    String *font_str = cat("pack/paint/tic_80_wide_font.wad");
    Wad *font_wad = wad_parse(font_str).wad;
    wad_delete(font_wad);

    String *map = cat("pack/maps/home.wad");
    game_open(game->game, map);
    string_delete(map);

    Image *font = new_image();
    image_delete(font);

    char *error = hymn_call(game->vm, "load", 0);
    if (error != NULL) {
        fprintf(stderr, error);
        fflush(stderr);
        exit(1);
    }
}

static void window_update(Window *win) {
    SDL_UpdateTexture(win->texture, NULL, win->canvas->pixels, win->canvas->width * sizeof(u32));
    SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
}

static void main_loop(Main *game) {
    Window *win = game->win;
    SDL_Renderer *renderer = win->renderer;

    u32 time = SDL_GetTicks();

    while (run) {
        poll_events(&game->input);

        game->state->update(game->state);
        game->state->draw(game->state);

        window_update(win);

        sleeping(time);
        SDL_RenderPresent(renderer);

        time = SDL_GetTicks();
    }
}

static void main_delete(Main *game) {
    hymn_delete(game->vm);
    free(game->win->canvas->pixels);
    free(game->win->canvas);
    free(game->win);
    free(game);
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
    HymnString *content = cat(string);
    HymnObjectString *object = hymn_new_string_object(content);
    return hymn_new_string_value(object);
}

int main(int argc, char **argv) {
    (int)argc;
    (void *)argv;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    window_init(&window, &renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    u32 pixel_format = SDL_PIXELFORMAT_ARGB8888;
    SDL_Texture *texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    Canvas *canvas = new_canvas(SCREEN_WIDTH, SCREEN_HEIGHT);

    Window *win = safe_calloc(1, sizeof(Window));
    win->window = window;
    win->renderer = renderer;
    win->texture = texture;
    win->canvas = canvas;

    Hymn *vm = new_hymn();

    // hymn_import_path("src\\<path>.hm");

    char *error = hymn_read(vm, "src/main.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    hymn_add_function(vm, "read_file", read_file);
    hymn_add_function(vm, "graphics_rect", canvas_rectangle_vm);
    hymn_add_pointer(vm, "canvas", canvas);

    Assets *assets = new_assets();

    Main *game = safe_calloc(sizeof(Main), 1);
    game->win = win;
    game->vm = vm;
    game->game = new_game(canvas, game->vm, &game->input, assets);
    game->paint = new_paint(canvas, &game->input, assets);
    switch_state(game, game->game);

    main_load(game);

    SDL_StartTextInput();

    main_loop(game);

    SDL_StopTextInput();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    main_delete(game);
    assets_delete(assets);

    printf("exiting...\n");
    fflush(stdout);

    return 0;
}
