#ifndef PIE_H
#define PIE_H

#include <inttypes.h>

#define is ==
#define isnt !=
#define not(expr) !(expr)
#define and &&
#define or ||

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;

#define null ((void *)0)

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#define PACK(expr) expr __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define UNUSED
#define PACK(expr) __pragma(pack(push, 1)) expr __pragma(pack(pop))
#endif

#endif