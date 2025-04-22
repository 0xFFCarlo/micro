#ifndef NOISE_H
#define NOISE_H

#include <stdbool.h>

typedef struct NoiseCfg
{
  int width;
  int height;
  float frequency;
  float amplitude;
  float offsetX;
  float offsetY;
  int octaves;
  float persistence;
  float lacunarity;
  bool usePeriodic;
  int periodX;
  int periodY;
  bool normalize;
} NoiseCfg;

void microNoisePelin2(float *data, const NoiseCfg cfg);

#endif /* NOISE_H */
