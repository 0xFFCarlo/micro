#ifndef PLANET_H
#define PLANET_H

#define GROUND_HEIGHT 8

extern void PlanetEntityAdd();
extern float PlanetGetRadius();
extern void PlanetGetPos(float *x, float *y);
extern void PlanetGetSurfacePosition(float angle, float offset, int *x, int *y);

#endif /* end of include guard: PLANET_H */
