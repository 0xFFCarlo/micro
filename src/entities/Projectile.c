#include "Projectile.h"
#include "../components/CustomComponents.h"
#include "../micro/components/LogicComponents.h"
#include "../micro/components/MotionComponents.h"
#include "../micro/components/RenderingComponents.h"
#include "../micro/core/ECS.h"
#include "../micro/core/Graphics.h"
#include "../micro/core/Physics.h"
#include "../micro/core/Resources.h"
#include "../misc/collision.h"
#include "../misc/layers.h"
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

  // Remove projectile on collision
  microECSEntityRemove(entityId);
}

void ProjectileAddEntity(ProjectileType type, const int x, const int y, const int vx,
                                const int vy)
{
  int projectile_entity_id = microECSEntityNew(NULL, ProjectileFree);
  assert(projectile_entity_id != -1);

  // Position component
  CmpAddPosition(projectile_entity_id, x, y);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);

  MicroTextureSource ts;
  if (type == PROJECTILE_TYPE_PLAYER)
    ts = microTextureAtlasGetRegion(atlasId, "laser-blue");
  else if (type == PROJECTILE_TYPE_ENEMY)
   ts = microTextureAtlasGetRegion(atlasId, "laser-red");

  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  CmpAddSprite(projectile_entity_id, textureId, ts.x, ts.y, ts.w, ts.h);

  const float rotation = atan2(vy, vx);
  CmpAddTransform(projectile_entity_id, PROJECTILE_SIZE * 2, PROJECTILE_SIZE,
                  PROJECTILE_SIZE / 2.0, PROJECTILE_SIZE / 2.0, rotation);
  CmpAddColor(projectile_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(projectile_entity_id, LAYER_EFFECTS, TRUE);

  // Body component
  int projectile_body_id = microPhysicsBodyNewCircle(projectile_entity_id, 0, x,
                                                     y, PROJECTILE_SIZE / 2.0,
                                                     0.0001, FALSE, FALSE, 0.0, 1.0);
  microPhysicsBodySetCollisionCallback(projectile_body_id, ProjectileCollision);

  if (type == PROJECTILE_TYPE_PLAYER)
    microPhysicsBodySetFilter(projectile_body_id, COLLISION_GROUP_PLAYER, COLLISION_MASK_PLAYER);
  else if (type == PROJECTILE_TYPE_ENEMY)
    microPhysicsBodySetFilter(projectile_body_id, COLLISION_GROUP_ENEMY, COLLISION_MASK_ENEMY);

  microPhysicsBodySetVelocity(projectile_body_id, vx, vy);
  CmpAddBody(projectile_entity_id, projectile_body_id);
  CmpAddProjectile(projectile_entity_id, 1);
  CmpAddLifetime(projectile_entity_id, 3.0);
}
