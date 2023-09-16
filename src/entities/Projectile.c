#include "Projectile.h"
#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "Explosion.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PROJECTILE_SIZE 8

void ProjectileFree(int entityId)
{
  CBody *body = CmpGetBody(entityId);
  microPhysicsBodyFree(body->body_id);
}

void ProjectileCollision(int entityId, int otherEntityId)
{
  (void)entityId;
  (void)otherEntityId;

  // Collision with projectile
  // if (microECSEntityHasComponent(otherEntityId, cid_health))
  // {
  //   CProjectile *p = microECSEntityGetComponent(entityId, cid_projectile);
  //   CHealth *health = microECSEntityGetComponent(otherEntityId, cid_health);
  //   health->health -= p->damage;
  //   health->health = health->health < 0 ? 0 : health->health;
  // }

  CPosition *pos = CmpGetPosition(entityId);
  CBody *body = CmpGetBody(entityId);
  float vx, vy;
  microPhysicsBodyGetVelocity(body->body_id, &vx, &vy);
  makeExplostionProjectileGroundHit(pos->x, pos->y, vx, vy);
  PlanetTryMine(pos->x, pos->y);

  microECSEntityRemove(entityId);
}

void ProjectileAddEntity(const int x, const int y, const int vx, const int vy)
{
  int projectile_entity_id = microECSEntityNew(NULL, ProjectileFree);
  assert(projectile_entity_id != -1);

  // Position component
  CmpAddPosition(projectile_entity_id, x, y);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "projectile-1");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  CmpAddSprite(projectile_entity_id, textureId, ts.x, ts.y, ts.w, ts.h);

  CmpAddTransform(projectile_entity_id, PROJECTILE_SIZE, PROJECTILE_SIZE,
                  PROJECTILE_SIZE / 2.0, PROJECTILE_SIZE / 2.0, 0.0);
  CmpAddColor(projectile_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(projectile_entity_id, 4, 1);

  // Body component
  int projectile_body_id = microPhysicsBodyNewCircle(projectile_entity_id, 0, x,
                                                     y, PROJECTILE_SIZE / 2.0,
                                                     0.0001, 0, 1.0, 0.0, 1.0);
  microPhysicsBodySetCollisionCallback(projectile_body_id, ProjectileCollision);
  microPhysicsBodySetFilter(projectile_body_id, 2, 1);
  microPhysicsBodySetVelocity(projectile_body_id, vx, vy);
  CmpAddBody(projectile_entity_id, projectile_body_id);
  CmpAddProjectile(projectile_entity_id, 1);
  CmpAddLifetime(projectile_entity_id, 4.0);
}
