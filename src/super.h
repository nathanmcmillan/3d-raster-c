/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SUPER_H
#define SUPER_H

#include <inttypes.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

#ifdef __GNUC__
#define PACK(expr) expr __attribute__((__packed__))
#elif _MSC_VER
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#endif

#endif
