/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "quaternion.h"

void quaternion_identity(quaternion *out) {
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 1.0f;
}

void euler_to_quaternion(quaternion *out, float x, float y, float z) {

    float cy = cosf(z * 0.5f);
    float sy = sinf(z * 0.5f);
    float cp = cosf(y * 0.5f);
    float sp = sinf(y * 0.5f);
    float cr = cosf(x * 0.5f);
    float sr = sinf(x * 0.5f);

    out[0] = sr * cp * cy - cr * sp * sy;
    out[1] = cr * sp * cy + sr * cp * sy;
    out[2] = cr * cp * sy - sr * sp * cy;
    out[3] = cr * cp * cy + sr * sp * sy;
}

void quaternion_to_euler(Vec3 *out, quaternion *in) {

    float sinr_cosp = 2.0f * (in[3] * in[0] + in[1] * in[2]);
    float cosr_cosp = 1.0f - 2.0f * (in[0] * in[0] + in[1] * in[1]);

    out->x = atan2f(sinr_cosp, cosr_cosp);

    float sinp = 2 * (in[3] * in[1] - in[2] * in[0]);

    if (sinp >= 1) {
        out->y = FLOAT_MATH_HALF_PI;
    } else if (sinp <= -1) {
        out->y = -FLOAT_MATH_HALF_PI;
    } else {
        out->y = asinf(sinp);
    }

    float siny_cosp = 2.0f * (in[3] * in[2] + in[0] * in[1]);
    float cosy_cosp = 1.0f - 2.0f * (in[1] * in[1] + in[2] * in[2]);

    out->z = atan2f(siny_cosp, cosy_cosp);
}

void quaternion_multiply(quaternion *out, quaternion *a, quaternion *b) {

    out[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    out[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
    out[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
    out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

float quaternion_dot(quaternion *a, quaternion *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

float quaternion_magnitude(quaternion *out) {
    return sqrtf(out[0] * out[0] + out[1] * out[1] + out[2] * out[2] + out[3] * out[3]);
}

void quaternion_normalize(quaternion *out) {

    float magnitude = quaternion_magnitude(out);
    float multiple = 1.0f / magnitude;
    out[0] *= multiple;
    out[1] *= multiple;
    out[2] *= multiple;
    out[3] *= multiple;
}

void quaternion_negate(quaternion *out, quaternion *in) {
    out[0] = -in[0];
    out[1] = -in[1];
    out[2] = -in[2];
    out[3] = -in[3];
}

void quaternion_conjugate(quaternion *out, quaternion *in) {
    out[0] = -in[0];
    out[1] = -in[1];
    out[2] = -in[2];
    out[3] = in[3];
}

void quaternion_scale(quaternion *out, quaternion *in, float scale) {
    out[0] = in[0] * scale;
    out[1] = in[1] * scale;
    out[2] = in[2] * scale;
    out[3] = in[3] * scale;
}

void quaternion_inverse(quaternion *out, quaternion *in) {
    float conjugate[4];
    quaternion_conjugate(conjugate, in);
    float magnitude_squared = in[0] * in[0] + in[1] * in[1] + in[2] * in[2] + in[3] * in[3];
    quaternion_scale(out, conjugate, 1.0f / magnitude_squared);
}

void quaternion_lerp(quaternion *out, quaternion *a, quaternion *b, float lerp) {

    out[0] = a[0] + lerp * (b[0] - a[0]);
    out[1] = a[1] + lerp * (b[1] - a[1]);
    out[2] = a[2] + lerp * (b[2] - a[2]);
    out[3] = a[3] + lerp * (b[3] - a[3]);

    quaternion_normalize(out);
}

void rotation_and_position_to_matrix(float *matrix, quaternion *rotation, float *position) {

    float magnitude = quaternion_magnitude(rotation);

    float s = magnitude > 0.0f ? 2.0f / magnitude : 0.0f;

    float x = rotation[0];
    float y = rotation[1];
    float z = rotation[2];
    float w = rotation[3];

    float xx = s * x * x;
    float xy = s * x * y;
    float wx = s * w * x;
    float yy = s * y * y;
    float yz = s * y * z;
    float wy = s * w * y;
    float zz = s * z * z;
    float xz = s * x * z;
    float wz = s * w * z;

    matrix[0] = 1.0f - yy - zz;
    matrix[1] = xy - wz;
    matrix[2] = xz + wy;
    matrix[3] = 0.0f;

    matrix[4] = xy + wz;
    matrix[5] = 1.0f - xx - zz;
    matrix[6] = yz - wx;
    matrix[7] = 0.0f;

    matrix[8] = xz - wy;
    matrix[9] = yz + wx;
    matrix[10] = 1.0f - xx - yy;
    matrix[11] = 0.0f;

    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    matrix[15] = 1.0f;
}
