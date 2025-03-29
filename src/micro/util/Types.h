#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef unsigned char u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef struct MicroAABBf
{
  f32 left, right, top, bottom;
} MicroAABBf;

typedef struct MicroAABB
{
  i32 left, right, top, bottom;
} MicroAABB;

typedef enum MicroDirection
{
  DIR_NONE = 0,
  DIR_UP = 1,
  DIR_DOWN = 2,
  DIR_LEFT = 3,
  DIR_RIGHT = 4,
} MicroDirection;

#define BIT(n) (1 << (n))

#endif // TYPES_H
