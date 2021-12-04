/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef GAME_H
#define GAME_H

#include "draw.h"
#include "state.h"

Game *new_game(Hymn *vm, Input *input);
void game_open(Game *this, String *content);
void game_update(void *state);
void game_draw(void *state);
void game_delete(Game *this);

#endif
