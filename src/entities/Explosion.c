#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../util/vector2d.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float projectileSpeed = 1000.0;
float projectileVx = 0.0;
float projectileVy = 0.0;

MicroParticle particleGroundHit(int emitterId)
{
  MicroParticle p;
  p.rotation = 0;
  p.rotationSpeed = 0;
  p.maxLife = 0.5;
  p.startAlpha = 1.0;
  p.endAlpha = 0.0;
  const int scale = 6 + rand() % 12;
  p.startScale = scale;
  p.endScale = scale;

  int peX, peY;
  microParticleEmitterGetPosition(emitterId, &peX, &peY);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  Vector2D planetNorm = {planetX - peX, planetY - peY};
  planetNorm = v2d_normalize(planetNorm);

  Vector2D projectileVec = {projectileVx, projectileVy};
  projectileVec = v2d_normalize(projectileVec);

  Vector2D dir = v2d_reflect(projectileVec, planetNorm);
  float angle = atan2(dir.y, dir.x);
  float angleDev = (M_PI) * ((float)rand() / (float)RAND_MAX) - M_PI / 2.0;
  angle += angleDev;
  dir.x = cos(angle);
  dir.y = sin(angle);

  p.vx = dir.x * projectileSpeed * 0.5;
  p.vy = dir.y * projectileSpeed * 0.5;

  int atlasId = microResourceGet("atlas");
  assert(atlasId != -1);
  int textureId = microTextureAtlasGetTextureId(atlasId);
  assert(textureId != -1);
  assert(textureId < 100);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId,
                                                     "particle-ground");
  p.textureId = textureId;
  p.tx = ts.x;
  p.ty = ts.y;
  p.tw = ts.w;
  p.th = ts.h;

  return p;
}

void makeExplostionProjectileGroundHit(int x, int y, float vx, float vy)
{
  // Store projectile info
  projectileSpeed = sqrt(vx * vx + vy * vy);
  projectileVx = vx;
  projectileVy = vy;

  int explosion_entity_id = microECSEntityNew(NULL, NULL);
  assert(explosion_entity_id != -1);
  CmpAddPosition(explosion_entity_id, x, y);
  CmpAddDrawable(explosion_entity_id, 4, 1);
  const int emitterId = microParticleEmitterCreateExplosion(x, y, 5,
                                                            particleGroundHit);
  assert(emitterId != -1);
  CmpAddParticleEmitter(explosion_entity_id, emitterId, 0, 0);
  CmpAddLifetime(explosion_entity_id, 1.0);
}

MicroParticle particleDroneExplodes(int emitterId)
{
  MicroParticle p;
  p.rotation = 0;
  p.rotationSpeed = 0;
  p.maxLife = 1.0;
  p.startAlpha = 1.0;
  p.endAlpha = 0.0;
  const int scale = 2 + rand() % 6;
  p.startScale = scale;
  p.endScale = scale;

  int peX, peY;
  microParticleEmitterGetPosition(emitterId, &peX, &peY);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  Vector2D planetNorm = {planetX - peX, planetY - peY};
  planetNorm = v2d_normalize(planetNorm);

  Vector2D projectileVec = {projectileVx, projectileVy};
  projectileVec = v2d_normalize(projectileVec);

  Vector2D dir = v2d_reflect(projectileVec, planetNorm);
  // float angle = -atan2(dir.y, dir.x);
  // float angleDev = (M_PI/2) * ((float)rand() / (float)RAND_MAX) - M_PI / 4.0;
  // angle += angleDev;
  float angle = ((float)rand() / (float)RAND_MAX) * 2.0 * M_PI;
  dir.x = cos(angle);
  dir.y = sin(angle);

  float randf = ((float)rand() / (float)RAND_MAX);
  p.vx = dir.x * projectileSpeed * (0.1 + 0.6 * randf);
  p.vy = dir.y * projectileSpeed * (0.1 + 0.6 * randf);

  int atlasId = microResourceGet("atlas");
  assert(atlasId != -1);
  int textureId = microTextureAtlasGetTextureId(atlasId);
  assert(textureId != -1);
  assert(textureId < 100);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "particle-smoke");
  p.textureId = textureId;
  p.tx = ts.x;
  p.ty = ts.y;
  p.tw = ts.w;
  p.th = ts.h;

  return p;
}

void makeExplosionDroneHit(int x, int y, float vx, float vy, int particleCount,
                           int explode)
{
  // Store projectile info
  projectileSpeed = sqrt(vx * vx + vy * vy);
  projectileVx = vx;
  projectileVy = vy;

  if (explode)
  {
    int explosion_id = microECSEntityNew(NULL, NULL);
    assert(explosion_id != -1);
    CmpAddPosition(explosion_id, x, y);
    int atlasId = microResourceGet("atlas");
    assert(atlasId != -1);
    int textureId = microTextureAtlasGetTextureId(atlasId);
    CmpAddSprite(explosion_id, textureId, 0, 0, 48, 48);
    f32 rangle = ((float)rand() / (float)RAND_MAX) * 2.0 * M_PI;
    CmpAddTransform(explosion_id, 128, 128, 64, 64, rangle);
    int animation_id = microAnimationGet("explosion-1");
    CmpAddAnimation(explosion_id, animation_id, 0.5, FALSE, FALSE, FALSE);
    CmpAddDrawable(explosion_id, 4, 1);
    CmpAddLifetime(explosion_id, 0.5);
  }
  int explosion_parts_id = microECSEntityNew(NULL, NULL);
  assert(explosion_parts_id != -1);
  CmpAddPosition(explosion_parts_id, x, y);

  CmpAddDrawable(explosion_parts_id, 4, 1);
  const int
    emitterId = microParticleEmitterCreateExplosion(x, y, particleCount,
                                                    particleDroneExplodes);
  assert(emitterId != -1);
  CmpAddParticleEmitter(explosion_parts_id, emitterId, 0, 0);
  CmpAddLifetime(explosion_parts_id, 1.0);
}
