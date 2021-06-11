/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "triangle.h"

Triangle *new_triangle(float height, int texture, Vec va, Vec vb, Vec vc, bool floor, float scale) {
    Triangle *td = safe_malloc(sizeof(Triangle));
    td->height = height;
    td->texture = texture;
    td->va = va;
    td->vb = vb;
    td->vc = vc;
    td->u1 = va.x * scale;
    td->v1 = va.y * scale;
    td->u2 = vb.x * scale;
    td->v2 = vb.y * scale;
    td->u3 = vc.x * scale;
    td->v3 = vc.y * scale;
    td->normal = floor ? 1.0 : -1.0;
    return td;
}
