/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <stdio.h>

#include "math_util.h"

#include "vec.h"

void matrix_print(float *matrix);
void matrix_identity(float *matrix);
void matrix_orthographic(float *matrix, float left, float right, float bottom, float top, float near, float far);
void matrix_frustum(float *matrix, float left, float right, float bottom, float top, float near, float far);
void matrix_perspective(float *matrix, float fov, float near, float far, float aspect);
void matrix_translate(float *matrix, float x, float y, float z);
void matrix_set_translation(float *matrix, float x, float y, float z);
void matrix_multiply(float *matrix, float *a, float *b);
void matrix_rotate_x(float *matrix, float sine, float cosine);
void matrix_rotate_y(float *matrix, float sine, float cosine);
void matrix_rotate_z(float *matrix, float sine, float cosine);
void matrix_inverse(float *matrix, float *from);
void matrix_transpose(float *matrix, float *from);
void matrix_multiply_vector3(float *out, float *matrix, float *vec);
void matrix_multiply_vector4(Vec4 *out, float *matrix, Vec4 *vec);
void matrix_look_at(float *matrix, Vec3 *eye, Vec3 *center);
void matrix_frustum_planes(float *frustum, float *matrix);
void matrix_frustum_corners(Vec4 *corners, float *matrix);
void matrix_orthographic_projection(float *mvp, float *orthographic, float *mv, float x, float y);
void matrix_perspective_projection(float *mvp, float *perspective, float *mv, float x, float y, float z, float rx, float ry);

#endif
