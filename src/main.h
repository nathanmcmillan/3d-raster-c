/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef MAIN_H
#define MAIN_H

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
typedef struct Game Game;

struct Window {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Canvas *canvas;
};

struct Game {
    Window *win;
    Hymn *vm;
    Input input;
    GameState *game;
    PaintState *paint;
    State *state;
};

#endif
