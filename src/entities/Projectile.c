#include "Projectile.h"
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

#define PROJECTILE_SIZE 8

void ProjectileFree(int entityId)
{
  CBody *body = microECSEntityGetComponent(entityId, cid_body);
  microPhysicsBodyFree(body->body_id);
}

void ProjectileCollision(int entityId, int otherEntityId)
{
  // Collision with projectile
  if (microECSEntityHasComponent(otherEntityId, cid_health))
  {
    CProjectile *p = microECSEntityGetComponent(entityId, cid_projectile);
    CHealth *health = microECSEntityGetComponent(otherEntityId, cid_health);
    health->health -= p->damage;
    health->health = health->health < 0 ? 0 : health->health;
  }

  microECSEntityRemove(entityId);
}

void ProjectileAddEntity(const int x, const int y, const int vx, const int vy)
{
  (void)(vx); // Unused parameter
  (void)(vy); // Unused parameter

  int projectile_entity_id = microECSEntityNew(NULL, ProjectileFree);
  assert(projectile_entity_id != -1);

  // Position component
  microECSEntityAddComponent(projectile_entity_id, cid_position,
                             &(CPosition){
                               .x = x,
                               .y = y,
                             });

  // Sprite component
  // int textureId = microTextureLoadFromFile("./res/robot.png");
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "projectile-1");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  microECSEntityAddComponent(projectile_entity_id, cid_sprite,
                             &(CSprite){
                               .textureId = textureId,
                               .tx = ts.x,
                               .ty = ts.y,
                               .tw = ts.w,
                               .th = ts.h,
                             });

  // Transform component
  microECSEntityAddComponent(projectile_entity_id, cid_transform,
                             &(CTransform){
                               .width = PROJECTILE_SIZE,
                               .height = PROJECTILE_SIZE,
                               .originX = PROJECTILE_SIZE / 2.0,
                               .originY = PROJECTILE_SIZE / 2.0,
                               .rotation = 0.0,
                             });

  // Color component
  microECSEntityAddComponent(projectile_entity_id, cid_color,
                             &(CColor){
                               .r = 1.0,
                               .g = 1.0,
                               .b = 1.0,
                               .a = 1.0,
                             });

  // Layer component
  microECSEntityAddComponent(projectile_entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = 4,
                               .visible = 1,
                             });

  // Body component
  int projectile_body_id = microPhysicsBodyNewCircle(projectile_entity_id, 0, x,
                                                     y, PROJECTILE_SIZE / 2.0,
                                                     0.0001, 0, 1.0, 0.0, 1.0);
  microPhysicsBodySetCollisionCallback(projectile_body_id, ProjectileCollision);

  microECSEntityAddComponent(projectile_entity_id, cid_body,
                             &(CBody){
                               .body_id = projectile_body_id,
                             });

  // Projectile component
  microECSEntityAddComponent(projectile_entity_id, cid_projectile,
                             &(CProjectile){
                               .damage = 1,
                             });

  // Lifetime
  microECSEntityAddComponent(projectile_entity_id, cid_lifetime,
                             &(CLifetime){
                               .lifetime = 4.0,
                             });

  // TODO:: gravity
}
