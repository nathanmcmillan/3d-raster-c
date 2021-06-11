/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "matrix.h"

void matrix_print(float *matrix) {
    printf("[");
    for (int i = 0; i < 15; i++) {
        printf("%f, ", matrix[i]);
    }
    printf("%f]\n", matrix[15]);
}

void matrix_identity(float *matrix) {

    matrix[0] = 1.0;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;

    matrix[4] = 0.0;
    matrix[5] = 1.0;
    matrix[6] = 0.0;
    matrix[7] = 0.0;

    matrix[8] = 0.0;
    matrix[9] = 0.0;
    matrix[10] = 1.0;
    matrix[11] = 0.0;

    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = 0.0;
    matrix[15] = 1.0;
}

void matrix_orthographic(float *matrix, float left, float right, float bottom, float top, float near, float far) {

    matrix[0] = 2.0 / (right - left);
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;

    matrix[4] = 0.0;
    matrix[5] = 2.0 / (top - bottom);
    matrix[6] = 0.0;
    matrix[7] = 0.0;

    matrix[8] = 0.0;
    matrix[9] = 0.0;
    matrix[10] = -2.0 / (far - near);
    matrix[11] = 0.0;

    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(far + near) / (far - near);
    matrix[15] = 1.0;
}

void matrix_frustum(float *matrix, float left, float right, float bottom, float top, float near, float far) {

    matrix[0] = (2.0 * near) / (right - left);
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;

    matrix[4] = 0.0;
    matrix[5] = (2.0 * near) / (top - bottom);
    matrix[6] = 0.0;
    matrix[7] = 0.0;

    matrix[8] = (right + left) / (right - left);
    matrix[9] = (top + bottom) / (top - bottom);
    matrix[10] = -(far + near) / (far - near);
    matrix[11] = -1.0;

    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = -(2.0 * far * near) / (far - near);
    matrix[15] = 0.0;
}

void matrix_perspective(float *matrix, float fov, float near, float far, float aspect) {

    float top = near * tanf(fov * MATH_PI / 360.0);
    float bottom = -top;
    float left = bottom * aspect;
    float right = top * aspect;

    matrix_frustum(matrix, left, right, bottom, top, near, far);
}

void matrix_translate(float *matrix, float x, float y, float z) {

    matrix[12] = x * matrix[0] + y * matrix[4] + z * matrix[8] + matrix[12];
    matrix[13] = x * matrix[1] + y * matrix[5] + z * matrix[9] + matrix[13];
    matrix[14] = x * matrix[2] + y * matrix[6] + z * matrix[10] + matrix[14];
    matrix[15] = x * matrix[3] + y * matrix[7] + z * matrix[11] + matrix[15];
}

void matrix_multiply(float *matrix, float *a, float *b) {

    matrix[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    matrix[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    matrix[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    matrix[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

    matrix[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
    matrix[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
    matrix[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
    matrix[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];

    matrix[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
    matrix[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
    matrix[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
    matrix[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];

    matrix[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
    matrix[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    matrix[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
    matrix[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
}

void matrix_rotate_x(float *matrix, float sine, float cosine) {

    float temp[16];

    temp[0] = 1.0;
    temp[1] = 0.0;
    temp[2] = 0.0;
    temp[3] = 0.0;

    temp[4] = 0.0;
    temp[5] = cosine;
    temp[6] = sine;
    temp[7] = 0.0;

    temp[8] = 0.0;
    temp[9] = -sine;
    temp[10] = cosine;
    temp[11] = 0.0;

    temp[12] = 0.0;
    temp[13] = 0.0;
    temp[14] = 0.0;
    temp[15] = 1.0;

    float copy[16];
    memcpy(copy, matrix, 16 * sizeof(float));

    matrix_multiply(matrix, copy, temp);
}

void matrix_rotate_y(float *matrix, float sine, float cosine) {

    float temp[16];

    temp[0] = cosine;
    temp[1] = 0.0;
    temp[2] = -sine;
    temp[3] = 0.0;

    temp[4] = 0.0;
    temp[5] = 1.0;
    temp[6] = 0.0;
    temp[7] = 0.0;

    temp[8] = sine;
    temp[9] = 0.0;
    temp[10] = cosine;
    temp[11] = 0.0;

    temp[12] = 0.0;
    temp[13] = 0.0;
    temp[14] = 0.0;
    temp[15] = 1.0;

    float copy[16];
    memcpy(copy, matrix, 16 * sizeof(float));

    matrix_multiply(matrix, copy, temp);
}

void matrix_rotate_z(float *matrix, float sine, float cosine) {

    float temp[16];

    temp[0] = cosine;
    temp[1] = sine;
    temp[2] = 0.0;
    temp[3] = 0.0;

    temp[4] = -sine;
    temp[5] = cosine;
    temp[6] = 0.0;
    temp[7] = 0.0;

    temp[8] = 0.0;
    temp[9] = 0.0;
    temp[10] = 1.0;
    temp[11] = 0.0;

    temp[12] = 0.0;
    temp[13] = 0.0;
    temp[14] = 0.0;
    temp[15] = 1.0;

    float copy[16];
    memcpy(copy, matrix, 16 * sizeof(float));

    matrix_multiply(matrix, copy, temp);
}

void matrix_set_translation(float *matrix, float x, float y, float z) {

    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void matrix_inverse(float *matrix, float *from) {

    float src[16];

    src[0] = from[0];
    src[4] = from[1];
    src[8] = from[2];
    src[12] = from[3];

    src[1] = from[4];
    src[5] = from[5];
    src[9] = from[6];
    src[13] = from[7];

    src[2] = from[8];
    src[6] = from[9];
    src[10] = from[10];
    src[14] = from[11];

    src[3] = from[12];
    src[7] = from[13];
    src[11] = from[14];
    src[15] = from[15];

    float tmp[16];

    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];

    float dst[16];

    dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
    dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
    dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
    dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
    dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];

    dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
    dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
    dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
    dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
    dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
    dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
    dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
    dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
    dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
    dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
    dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
    dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
    dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

    float det = 1.0 / (src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3]);

    for (int i = 0; i < 16; i++) {
        matrix[i] = dst[i] * det;
    }
}

void matrix_transpose(float *matrix, float *from) {

    matrix[0] = from[0];
    matrix[4] = from[1];
    matrix[8] = from[2];
    matrix[12] = from[3];

    matrix[1] = from[4];
    matrix[5] = from[5];
    matrix[9] = from[6];
    matrix[13] = from[7];

    matrix[2] = from[8];
    matrix[6] = from[9];
    matrix[10] = from[10];
    matrix[14] = from[11];

    matrix[3] = from[12];
    matrix[7] = from[13];
    matrix[11] = from[14];
    matrix[15] = from[15];
}

void matrix_multiply_vector3(float *out, float *matrix, float *vec) {

    out[0] = vec[0] * matrix[0] + vec[1] * matrix[4] + vec[2] * matrix[8] + matrix[12];
    out[1] = vec[0] * matrix[1] + vec[1] * matrix[5] + vec[2] * matrix[9] + matrix[13];
    out[2] = vec[0] * matrix[2] + vec[1] * matrix[6] + vec[2] * matrix[10] + matrix[14];
}

void matrix_multiply_vector4(Vec4 *out, float *matrix, Vec4 *vec) {

    out->x = vec->x * matrix[0] + vec->y * matrix[4] + vec->z * matrix[8] + vec->w * matrix[12];
    out->y = vec->x * matrix[1] + vec->y * matrix[5] + vec->z * matrix[9] + vec->w * matrix[13];
    out->z = vec->x * matrix[2] + vec->y * matrix[6] + vec->z * matrix[10] + vec->w * matrix[14];
    out->w = vec->x * matrix[3] + vec->y * matrix[7] + vec->z * matrix[11] + vec->w * matrix[15];
}

void matrix_look_at(float *matrix, Vec3 *eye, Vec3 *center) {

    Vec3 forward = {
        center->x - eye->x,
        center->y - eye->y,
        center->z - eye->z,
    };
    vector3_normalize(&forward);

    Vec3 any = {0, 1, 0};

    Vec3 side;
    VECTOR_3_CROSS(side, forward, any);

    Vec3 up;
    VECTOR_3_CROSS(up, side, forward);

    matrix[0] = side.x;
    matrix[4] = side.y;
    matrix[8] = side.z;
    matrix[12] = 0;

    matrix[1] = up.x;
    matrix[5] = up.y;
    matrix[9] = up.z;
    matrix[13] = 0;

    matrix[2] = -forward.x;
    matrix[6] = -forward.y;
    matrix[10] = -forward.z;
    matrix[14] = 0;

    matrix[3] = 0;
    matrix[7] = 0;
    matrix[11] = 0;
    matrix[15] = 1;
}

void matrix_frustum_planes(float *frustum, float *matrix) {

    // Left
    frustum[0] = matrix[3] + matrix[0];
    frustum[1] = matrix[7] + matrix[4];
    frustum[2] = matrix[11] + matrix[8];
    frustum[3] = matrix[15] + matrix[12];

    // Right
    frustum[4] = matrix[3] - matrix[0];
    frustum[5] = matrix[7] - matrix[4];
    frustum[6] = matrix[11] - matrix[8];
    frustum[7] = matrix[15] - matrix[12];

    // Top
    frustum[8] = matrix[3] - matrix[1];
    frustum[9] = matrix[7] - matrix[5];
    frustum[10] = matrix[11] - matrix[9];
    frustum[11] = matrix[15] - matrix[13];

    // Bottom
    frustum[12] = matrix[3] + matrix[1];
    frustum[13] = matrix[7] + matrix[5];
    frustum[14] = matrix[11] + matrix[9];
    frustum[15] = matrix[15] + matrix[13];

    // Near
    frustum[16] = matrix[3] + matrix[2];
    frustum[17] = matrix[7] + matrix[6];
    frustum[18] = matrix[11] + matrix[10];
    frustum[19] = matrix[15] + matrix[14];

    // Far
    frustum[20] = matrix[3] - matrix[2];
    frustum[21] = matrix[7] - matrix[6];
    frustum[22] = matrix[11] - matrix[10];
    frustum[23] = matrix[15] - matrix[14];

    for (int i = 0; i < 6; i++) {
        float x = frustum[i * 4];
        float y = frustum[i * 4 + 1];
        float z = frustum[i * 4 + 2];
        float n = sqrtf(x * x + y * y + z * z);
        frustum[i * 4] /= n;
        frustum[i * 4 + 1] /= n;
        frustum[i * 4 + 2] /= n;
        frustum[i * 4 + 3] /= n;
    }
}

void matrix_frustum_corners(Vec4 *corners, float *matrix) {

    corners[0] = (Vec4){-1, -1, -1, 1}; // Left  Bottom Near
    corners[1] = (Vec4){1, -1, -1, 1};  // Right Bottom Near
    corners[2] = (Vec4){-1, 1, -1, 1};  // Left  Top    Near
    corners[3] = (Vec4){1, 1, -1, 1};   // Right Top    Near
    corners[4] = (Vec4){-1, -1, 1, 1};  // Left  Bottom Far
    corners[5] = (Vec4){1, -1, 1, 1};   // Right Bottom Far
    corners[6] = (Vec4){-1, 1, 1, 1};   // Left  Top    Far
    corners[7] = (Vec4){1, 1, 1, 1};    // Right Top    Far

    Vec4 transform;
    for (int i = 0; i < 8; i++) {
        matrix_multiply_vector4(&transform, matrix, &corners[i]);
        corners[i].x = transform.x / transform.w;
        corners[i].y = transform.y / transform.w;
        corners[i].z = transform.z / transform.w;
        corners[i].w = 1;
    }
}

void matrix_orthographic_projection(float *mvp, float *orthographic, float *mv, float x, float y) {

    matrix_identity(mv);
    matrix_translate(mv, x, y, 0);
    matrix_multiply(mvp, orthographic, mv);
}

void matrix_perspective_projection(float *mvp, float *perspective, float *mv, float x, float y, float z, float rx, float ry) {

    matrix_identity(mv);
    matrix_rotate_x(mv, sinf(rx), cosf(rx));
    matrix_rotate_y(mv, sinf(ry), cosf(ry));
    matrix_translate(mv, x, y, z);
    matrix_multiply(mvp, perspective, mv);
}
