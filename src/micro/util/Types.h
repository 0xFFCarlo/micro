#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct MicroAABBf
{
  float left, right, top, bottom;
} MicroAABBf;

typedef struct MicroAABB
{
  int32_t left, right, top, bottom;
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
