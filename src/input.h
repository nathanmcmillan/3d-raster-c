/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

struct Input {
    int mouse_x;
    int mouse_y;
    bool mouse_down;
    bool move_left;
    bool move_right;
    bool move_up;
    bool move_down;
    bool move_forward;
    bool move_backward;
    bool look_up;
    bool look_down;
    bool look_left;
    bool look_right;
    bool console;
};

typedef struct Input Input;

#endif
