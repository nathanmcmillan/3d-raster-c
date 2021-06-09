#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "canvas.h"
#include "fileio.h"
#include "pie.h"
#include "texture.h"
#include "wad.h"

typedef struct Window Window;
typedef struct Game Game;

struct Window {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Canvas *canvas;
};

struct Game {
    Window *win;
    lua_State *vm;
};

#endif
