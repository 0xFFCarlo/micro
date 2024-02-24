#include "Meteor.h"
#include "../components/CustomComponents.h"
#include "../micro/components/LogicComponents.h"
#include "../micro/components/MotionComponents.h"
#include "../micro/components/RenderingComponents.h"
#include "../micro/core/Audio.h"
#include "../micro/core/ECS.h"
#include "../micro/core/Graphics.h"
#include "../micro/core/Physics.h"
#include "../micro/core/Resources.h"
#include "../misc/collision.h"
#include "Explosion.h"
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define METEOR_SIZE 64
#define METEOR_BODY_RADIUS 32

void MeteorCollision(int entityId, int otherEntityId)
{
  (void)otherEntityId;

  // Remove on collision
  CPosition *position = CmpGetPosition(entityId);
  makeExplosionMeteorHit(position->x, position->y);
  microECSEntityRemove(entityId);
}

void MeteorFree(int entityId)
{
  CBody *body = CmpGetBody(entityId);
  microPhysicsBodyFree(body->body_id);
}

void MeteorAddEntity(const int x, const int y, const float speed)
{
  int meteor_id = microECSEntityNew(NULL, MeteorFree);
  assert(meteor_id != -1);
  CmpAddPosition(meteor_id, x, y);
  const int atlasId = microResourceGet("atlas");
  const int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "meteor-2");
  CmpAddSprite(meteor_id, textureId, ts.x, ts.y, ts.w, ts.h);
  CmpAddTransform(meteor_id, METEOR_SIZE, METEOR_SIZE, METEOR_SIZE / 2.0, METEOR_SIZE / 2.0, 0.0);
  CmpAddColor(meteor_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(meteor_id, 4, TRUE);
  int meteor_body_id = microPhysicsBodyNewCircle(meteor_id, 0, x, y, METEOR_BODY_RADIUS,
                                               0.01, FALSE, FALSE, 0, 0.0);
  microPhysicsBodySetCollisionCallback(meteor_body_id, MeteorCollision);
  microPhysicsBodySetFilter(meteor_body_id, COLLISION_GROUP_WORLD,
                            COLLISION_MASK_ALL);
  CmpAddBody(meteor_id, meteor_body_id);

  // Planetary alignment component
  f32 planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(meteor_id, planetX, planetY);

  // Set direction towards planet
  f32 dx = planetX - x;
  f32 dy = planetY - y;
  f32 dist = sqrt(dx * dx + dy * dy);
  dx /= dist;
  dy /= dist;
  microPhysicsBodySetVelocity(meteor_body_id, dx * speed, dy * speed);
}
