#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../misc/collision.h"
#include "Drone.h"
#include "Explosion.h"
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DRONE_SPEED 0.5
#define DRONE_TEX_WIDTH 64
#define DRONE_TEX_HEIGHT 64
#define DRONE_SCALE 2.0
#define DRONE_PROJ_SPEED 600.0

int aid_drone_2_fall = -1;
int aid_drone_2_walk = -1;

typedef struct DroneLanderData
{
  u32 is_falling;
} DroneLanderData;

void DroneLanderCollision(int entityId, int otherEntityId)
{
  (void)entityId;
  (void)otherEntityId;

  CBody *bodyOther = CmpGetBody(otherEntityId);
  f32 vx, vy;
  microPhysicsBodyGetVelocity(bodyOther->body_id, &vx, &vy);
  f32 speed = sqrt(vx * vx + vy * vy);
  if (speed > 300)
  {
    CHealth *health = CmpGetHealth(entityId);
    health->health -= 1;
    CPosition *position = CmpGetPosition(entityId);
    if (health->health <= 0.0)
    {
      makeExplosionDroneHit(position->x, position->y, vx, vy, 20, TRUE);
      uint32_t explosion_snd = microResourceGet("explosion");
      microSoundPlay(explosion_snd, 0);
      health->health = 0;
      microECSEntityRemove(entityId);
    }
    else
    {
      makeExplosionDroneHit(position->x, position->y, vx, vy, 3, FALSE);
    }
  }
}

void DroneLanderUpdate(int droneId, float dt)
{
  (void)dt;

  f32 playerX, playerY;
  PlayerGetPos(&playerX, &playerY);

  CPosition *position = CmpGetPosition(droneId);
  CAnimation *animation = CmpGetAnimation(droneId);

  // Get planet info
  f32 planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  f32 planetRadius = PlanetGetRadius();
  f32 planetDist = sqrt((planetX - position->x) * (planetX - position->x) +
                        (planetY - position->y) * (planetY - position->y));

  // Switch state from falling to walking on surface
  DroneLanderData *data = microECSEntityGetData(droneId);
  if (data->is_falling)
  {
    if (planetDist - planetRadius < DRONE_TEX_WIDTH / 4.0)
    {
      data->is_falling = FALSE;
      animation->animationId = aid_drone_2_walk;
      animation->frameId = 0;
      animation->timeSinceLastFrame = 0;
    }
  }

  // Shoot player if not falling and close enough
  if (!data->is_falling)
  {
    f32 dx = playerX - position->x;
    f32 dy = playerY - position->y;
    f32 dist = sqrt(dx * dx + dy * dy);
    if (dist < 200.0)
    {
      if (rand() % 60 == 0)
      {
        f32 angle = atan2(dy, dx);
        f32 vx = cos(angle) * DRONE_PROJ_SPEED;
        f32 vy = sin(angle) * DRONE_PROJ_SPEED;
        ProjectileAddEntity(PROJECTILE_TYPE_ENEMY, position->x, position->y, vx,
                            vy);
        int laser_snd_id = microResourceGet("gun_shot");
        if (laser_snd_id != -1)
          microSoundPlayNewChannel(laser_snd_id, 0);
      }
    }
  }

  // Compute force to adjust position
  f32 playerAngle = atan2(playerY - planetY, playerX - planetX);
  const f32 angle = atan2(planetY - position->y, planetX - position->x);
  f32 vforce = 20.0 * (planetDist - planetRadius);
  vforce = fmin(vforce, 150.0);
  f32 vforceX = cos(angle) * vforce;
  f32 vforceY = sin(angle) * vforce;

  // Walk towards player if on surface (not falling)
  f32 hforceX = 0;
  f32 hforceY = 0;
  if (data->is_falling == FALSE)
  {
    f32 hangle = angle + M_PI / 2.0;
    f32 hforce = 40;

    // Handle wrap-around scenarios
    f32 difference = playerAngle - angle;
    f32 wrappedDifference;
    animation->flipX = TRUE;
    if (difference > 0)
    {
      wrappedDifference = difference - 2 * M_PI;
    }
    else
    {
      wrappedDifference = difference + 2 * M_PI;
    }

    // Choose the direction based on the shortest arc
    if (fabs(difference) < fabs(wrappedDifference))
    {
      if (difference < 0)
      {
        hforce *= -1.0;
        animation->flipX = FALSE;
      }
    }
    else
    {
      if (wrappedDifference < 0)
      {
        hforce *= -1.0;
        animation->flipX = FALSE;
      }
    }

    hforceX = cos(hangle) * hforce;
    hforceY = sin(hangle) * hforce;
  }

  const f32 forceX = vforceX + hforceX;
  const f32 forceY = vforceY + hforceY;

  CBody *body = CmpGetBody(droneId);
  microPhysicsBodySetVelocity(body->body_id, forceX, forceY);
}

void DroneLanderFree(int entityId)
{
  DroneLanderData *data = microECSEntityGetData(entityId);
  free(data);
  CBody *body = CmpGetBody(entityId);
  microPhysicsBodyFree(body->body_id);
}

void DroneLanderAddEntity(const int x, const int y)
{
  // Load animations if needed
  if (aid_drone_2_fall == -1)
  {
    aid_drone_2_fall = microAnimationGet("drone-2-fall");
    aid_drone_2_walk = microAnimationGet("drone-2-walk");
  }

  DroneLanderData *data = malloc(sizeof(DroneLanderData));
  data->is_falling = TRUE;
  int drone_eid = microECSEntityNew(data, DroneLanderFree);
  assert(drone_eid != -1);

  CmpAddPosition(drone_eid, x, y);

  // Sprite component
  // int textureId = microTextureLoadFromFile("./res/robot.png");
  const int atlasId = microResourceGet("atlas");
  const int textureId = microTextureAtlasGetTextureId(atlasId);
  CmpAddSprite(drone_eid, textureId, 0, 0, 0, 0);
  const f32 size = DRONE_TEX_WIDTH * DRONE_SCALE;
  CmpAddTransform(drone_eid, size, size, size / 2.0, size / 2.0, 0.0);
  CmpAddColor(drone_eid, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(drone_eid, 4, TRUE);
  CmpAddAnimation(drone_eid, aid_drone_2_fall, 0.4, FALSE, FALSE, FALSE);
  CmpAddUpdate(drone_eid, DroneLanderUpdate);

  const f32 bodyRadius = (DRONE_TEX_WIDTH / 2.0) * DRONE_SCALE / 2.0;
  int drone_bodyid = microPhysicsBodyNewCircle(drone_eid, 0, x, y, bodyRadius,
                                               0.01, FALSE, FALSE, 0, 0.0);
  microPhysicsBodySetCollisionCallback(drone_bodyid, DroneLanderCollision);
  microPhysicsBodySetFilter(drone_bodyid, COLLISION_GROUP_ENEMY,
                            COLLISION_MASK_ENEMY);
  CmpAddBody(drone_eid, drone_bodyid);
  CmpAddHealth(drone_eid, 5, 5);

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(drone_eid, planetX, planetY);
}
