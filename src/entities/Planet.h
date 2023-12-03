#ifndef PLANET_H
#define PLANET_H

#include "../micro/Types.h"

#define GROUND_HEIGHT 16

extern void PlanetEntityAdd(const float radius);
extern float PlanetGetRadius();
extern void PlanetGetPos(float *x, float *y);
extern void PlanetGetSurfacePosition(float angle, float offset, int *x, int *y);
extern void PlanetGenerate();

#endif /* end of include guard: PLANET_H */
