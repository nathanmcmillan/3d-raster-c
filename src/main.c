/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "main.h"

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

static void game_call(Game *game, char *call) {
    Hymn *vm = game->vm;
    String *error = hymn_call(vm, call);
    if (error != NULL) {
        fprintf(stderr, "Hymn call error: %s\n", error);
    }
}

static void game_update(Game *game) {
    state_update(game->state);
    game_call(game, "update");
}

static void game_draw(Game *game) {
    state_draw(game->state);
    game_call(game, "draw");
}

static void game_load(Game *game) {

    String *font_str = cat("pack/paint/tic_80_wide_font.wad");
    Wad *font_wad = wad_parse(font_str).wad;
    wad_delete(font_wad);

    String *map = cat("pack/maps/base.wad");
    game_state_open(game->game, map);
    string_delete(map);

    Paint *font = new_paint();
    paint_delete(font);

    game_call(game, "load");
}

static void window_update(Window *win) {
    SDL_UpdateTexture(win->texture, NULL, win->canvas->pixels, win->canvas->width * sizeof(u32));
    SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);
}

static void main_loop(Game *game) {
    Window *win = game->win;
    SDL_Renderer *renderer = win->renderer;
    Canvas *canvas = win->canvas;

    u32 time = SDL_GetTicks();

    while (run) {
        poll_events(&game->input);

        game_update(game);

        canvas_clear_color(canvas);
        game_draw(game);
        window_update(win);

        sleeping(time);
        SDL_RenderPresent(renderer);

        time = SDL_GetTicks();
    }
}

static void game_delete(Game *game) {
    hymn_delete(game->vm);
    free(game->win->canvas->pixels);
    free(game->win->canvas);
    free(game->win);
    free(game);
}

static void game_switch_state(Game *game, void *state) {
    game->state = (State *)state;
}

int main(int argc, char **argv) {
    (int)argc;
    (void *)argv;

#define HYMN

#ifndef HYMN
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
#endif

    Hymn *vm = new_hymn();

    // {
    //     char *error = hymn_repl(vm);
    //     if (error != NULL) {
    //         fprintf(stderr, "%s\n", error);
    //         exit(1);
    //     }
    // }

    char *error = hymn_read(vm, "test/test.hm");
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

#ifdef HYMN
    return 0;
#else

    // char *error = hymn_read(vm, "src/main.hm");
    // if (error != NULL) {
    //     fprintf(stderr, "Hymn read error: %s\n", error);
    //     exit(1);
    // }

    hymn_add_func(vm, "graphics", canvas_rect_vm);
    hymn_add_pointer(vm, "canvas", canvas);

    Assets *assets = new_assets();

    Game *game = safe_calloc(sizeof(Game), 1);
    game->win = win;
    game->vm = vm;
    game->game = new_game_state(canvas, &game->input, assets);
    game->paint = new_paint_state(canvas, &game->input, assets);
    game_switch_state(game, game->game);

    game_load(game);

    SDL_StartTextInput();

    main_loop(game);

    SDL_StopTextInput();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    game_delete(game);
    assets_delete(assets);

    return 0;
#endif
}
