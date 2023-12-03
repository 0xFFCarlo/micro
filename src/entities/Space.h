#ifndef SPACE_H
#define SPACE_H

extern void SpaceEntityAdd();

extern void SpaceWarpDriveStart();
extern int SpaceIsWarping();
extern void SpaceSetAtmosphereMaxIntensity(float intensity);
extern void SpaceSetAtmosphereDecay(float decay);
extern void SpaceSetAtmosphereColor(float r, float g, float b);
extern void SpaceSetStarfieldTh1(float threshold);
extern void SpaceSetStarfieldTh2(float threshold);
extern void SpaceApplyParameters();

#endif // SPACE_H
