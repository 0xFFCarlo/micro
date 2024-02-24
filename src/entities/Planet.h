#ifndef PLANET_H
#define PLANET_H

#include "../micro/util/Types.h"

typedef enum {
  PLANET_ASTEROID = 0,
  PLANET_RED_ROCKY = 1,
  PLANET_TYPES_COUNT = 2
} PlanetType;

#define GROUND_HEIGHT 16

// Should be called 1 time at the beginning of the game
extern void PlanetEntityAdd();

// Used to generate new planet
extern void PlanetGenerate();

extern float PlanetGetRadius();
extern void PlanetGetPos(float *x, float *y);
extern void PlanetGetSurfacePosition(float angle, float offset, int *x, int *y);

#endif /* end of include guard: PLANET_H */
