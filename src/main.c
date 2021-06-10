/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "main.h"

static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;

static bool run = true;

static void window_init(SDL_Window **win, SDL_Renderer **ren) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    Uint32 window_flags = SDL_WINDOW_SHOWN;
    SDL_Window *window = SDL_CreateWindow("Scroll and Sigil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    if (window == NULL) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        exit(1);
    }

    *win = window;

    Uint32 renderer_flags = SDL_RENDERER_TARGETTEXTURE;
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

static void poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT: run = false; break;
        case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: run = false; break;
            }
            break;
        }
        }
    }
}

static void game_call(Game *game, char *call) {
    lua_State *vm = game->vm;
    lua_getglobal(vm, call);
    int error = lua_pcall(vm, 0, 0, 0);
    if (error != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(vm, -1));
    }
    lua_pop(vm, lua_gettop(vm));
}

static void game_update(Game *game) {
    game_call(game, "update");
}

static void game_draw(Game *game) {
    game_call(game, "draw");
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
        poll_events();

        game_update(game);

        canvas_clear(canvas);
        game_draw(game);
        window_update(win);

        sleeping(time);
        SDL_RenderPresent(renderer);

        time = SDL_GetTicks();
    }
}

int main(int argc, char **argv) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    window_init(&window, &renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    u32 pixel_format = SDL_PIXELFORMAT_ARGB8888;
    SDL_Texture *texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    Canvas *canvas = safe_calloc(sizeof(Canvas), 1);
    canvas->width = SCREEN_WIDTH;
    canvas->height = SCREEN_HEIGHT;
    canvas->pixels = safe_calloc(sizeof(u32), SCREEN_WIDTH * SCREEN_HEIGHT);

    Window *win = safe_calloc(sizeof(Window), 1);
    win->window = window;
    win->renderer = renderer;
    win->texture = texture;
    win->canvas = canvas;

    lua_State *vm = luaL_newstate();
    luaL_openlibs(vm);

    int error = luaL_dofile(vm, "src/main.lua");
    if (error != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(vm, -1));
        exit(1);
    }

    static luaL_Reg graphics[] = {{"rect", vm_canvas_rect}, {NULL, NULL}};
    luaL_newlib(vm, graphics);
    lua_setglobal(vm, "graphics");

    lua_pushlightuserdata(vm, canvas);
    lua_setglobal(vm, "canvas");

    Game *game = safe_calloc(sizeof(Game), 1);
    game->win = win;
    game->vm = vm;

    String *font_str = cat("res/tic-80-wide-font.wad");
    Wad *font_wad = wad_parse(font_str);

    Texture *font = new_texture();

    game_call(game, "load");

    SDL_StartTextInput();

    main_loop(game);

    SDL_StopTextInput();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    lua_close(vm);

    free(canvas->pixels);
    free(canvas);
    free(win);
    free(game);

    return 0;
}
