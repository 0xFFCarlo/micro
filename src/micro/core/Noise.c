#include "Noise.h"
#include "../util/debug.h"
#include "../util/perlin_noise.h"
#include <math.h>

static float fractalNoise2(float x, float y, const NoiseCfg *cfg)
{
  float total = 0.0f;
  float frequency = cfg->frequency;
  float amplitude = cfg->amplitude;
  float maxAmplitude = 0.0f;

  for (int i = 0; i < cfg->octaves; ++i)
  {
    float nx = x * frequency;
    float ny = y * frequency;

    float noise = cfg->usePeriodic ? pnoise2(nx, ny, cfg->periodX, cfg->periodY)
                                   : noise2(nx, ny);

    total += noise * amplitude;
    maxAmplitude += amplitude;

    amplitude *= cfg->persistence;
    frequency *= cfg->lacunarity;
  }

  return total / maxAmplitude;
}

void microNoisePelin2(float *data, const NoiseCfg cfg)
{
  assert(cfg.width > 0);
  assert(cfg.height > 0);
  assert(cfg.frequency != 0.0f);
  assert(cfg.amplitude != 0.0f);
  assert(cfg.octaves >= 1);

  for (int y = 0; y < cfg.height; ++y)
  {
    for (int x = 0; x < cfg.width; ++x)
    {
      float fx = (x + cfg.offsetX) / (float)cfg.width;
      float fy = (y + cfg.offsetY) / (float)cfg.height;

      float noise = fractalNoise2(fx, fy, &cfg);

      if (cfg.normalize)
        noise = (noise + 1.0f) * 0.5f;

      data[x + y * cfg.width] = noise;
    }
  }
}
