#include "Cave.h"
#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MicroParticle smokeGenerator(int emitterId)
{
  MicroParticle p;
  p.rotation = 0;
  p.rotationSpeed = 0;
  p.maxLife = 5.0;
  p.startAlpha = 0.8;
  p.endAlpha = 0.0;
  const int scale = 4 + rand() % 8;
  p.startScale = scale;
  p.endScale = scale;

  int ex, ey;
  microParticleEmitterGetPosition(emitterId, &ex, &ey);
  float px, py;
  PlanetGetPos(&px, &py);
  float dirX = (float)(px - ex);
  float dirY = (float)(py - ey);
  float dirLen = sqrt(dirX * dirX + dirY * dirY);
  dirX /= dirLen;
  dirY /= dirLen;

  float speed = 10.0 + rand() % 10;
  p.vx = -dirX * speed;
  p.vy = -dirY * speed;

  int atlasId = microResourceGet("atlas");
  assert(atlasId != -1);
  int textureId = microTextureAtlasGetTextureId(atlasId);
  assert(textureId != -1);
  assert(textureId < 100);
  MicroTextureSource ts;
  float rval = (float)(rand() % 1000) / 1000.0;
  if (rval < 0.75)
    ts = microTextureAtlasGetRegion(atlasId, "particle-smoke");
  else
    ts = microTextureAtlasGetRegion(atlasId, "particle-fire");
  p.textureId = textureId;
  p.tx = ts.x;
  p.ty = ts.y;
  p.tw = ts.w;
  p.th = ts.h;

  return p;
}

void CaveAddEntity(const int x, const int y)
{
  int cave_entity_id = microECSEntityNew(NULL, NULL);
  assert(cave_entity_id != -1);

  // Position component
  microECSEntityAddComponent(cave_entity_id, cid_position,
                             &(CPosition){
                               .x = x,
                               .y = y,
                             });

  // Sprite component
  // int textureId = microTextureLoadFromFile("./res/robot.png");
  const int atlasId = microResourceGet("atlas");
  const int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "cave");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  microECSEntityAddComponent(cave_entity_id, cid_sprite,
                             &(CSprite){
                               .textureId = textureId,
                               .tx = ts.x,
                               .ty = ts.y,
                               .tw = ts.w,
                               .th = ts.h,
                             });

  // Transform component
  microECSEntityAddComponent(cave_entity_id, cid_transform,
                             &(CTransform){
                               .width = 32,
                               .height = 32,
                               .originX = 32 / 2.0,
                               .originY = 32 / 2.0,
                               .rotation = 0.0,
                             });

  // Color component
  microECSEntityAddComponent(cave_entity_id, cid_color,
                             &(CColor){
                               .r = 1.0,
                               .g = 1.0,
                               .b = 1.0,
                               .a = 1.0,
                             });

  // Layer component
  microECSEntityAddComponent(cave_entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = 4,
                               .visible = 1,
                             });

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  microECSEntityAddComponent(cave_entity_id, cid_planetary_alignment,
                             &(CPlanetaryAlignment){
                               .planet_x = planetX,
                               .planet_y = planetY,
                             });

  // Smoke emittter component
  int emitterId = microParticleEmitterCreateSteady(x, y, 10.0, smokeGenerator);
  microParticleEmitterSetSize(emitterId, 16, 8);
  microECSEntityAddComponent(cave_entity_id, cid_particle_emitter,
                             &(CParticleEmitter){
                               .emitterId = emitterId,
                               .offsetX = 0,
                               .offsetY = 0,
                             });
}
