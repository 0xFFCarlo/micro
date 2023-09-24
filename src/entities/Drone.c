#include "Drone.h"
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
#include "Planet.h"
#include "Player.h"
#include "Projectile.h"
#include "Explosion.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DRONE_SPEED 0.5
#define DRONE_TEX_WIDTH 48
#define DRONE_TEX_HEIGHT 48
#define DRONE_SCALE 2.0
#define DRONE_PROJ_SPEED 600.0

void DroneCollision(int entityId, int otherEntityId)
{
  CBody* bodyOther = CmpGetBody(otherEntityId);
  f32 vx, vy;
  microPhysicsBodyGetVelocity(bodyOther->body_id, &vx, &vy);
  f32 speed = sqrt(vx * vx + vy * vy);
  if (speed > 300) {
    CHealth* health = CmpGetHealth(entityId);
    health->health -= 1;
    CPosition* position = CmpGetPosition(otherEntityId);
    if (health->health <= 0.0) 
    {
      makeExplosionDroneHit(position->x, position->y, vx, vy, 20);
      health->health = 0;
      microECSEntityRemove(entityId);
    }
    else
    {
      makeExplosionDroneHit(position->x, position->y, vx, vy, 3);
    }
  }
}

void DroneUpdate(int droneId, float dt)
{
  (void)dt;

  f32 playerX, playerY;
  PlayerGetPos(&playerX, &playerY);

  CPosition *position = CmpGetPosition(droneId);
  f32 dx = playerX - position->x;
  f32 dy = playerY - position->y;
  f32 dist = sqrt(dx * dx + dy * dy);
  if (dist < 400.0)
  {
    if (rand() % 120 == 0)
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
  
  // Get planet info
  f32 planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  f32 planetRadius = PlanetGetRadius();
  f32 planetDist = sqrt((planetX - position->x) * (planetX - position->x) +
                        (planetY - position->y) * (planetY - position->y));

  // Get player angle
  f32 playerAngle = atan2(playerY - planetY, playerX - planetX);
  
  const f32 targetAltitude = planetRadius + 200.0;

  // Compute force to adjust position
  const f32 angle = atan2(planetY - position->y, planetX - position->x);
  f32 vforce = 10.0 * (planetDist - targetAltitude);
  vforce = fmin(vforce, 100.0);
  f32 vforceX = cos(angle) * vforce;
  f32 vforceY = sin(angle) * vforce;
  
  // horizontal force
  f32 hangle = angle + M_PI / 2.0;
  f32 hforce = 60;

  // Handle wrap-around scenarios
  f32 difference = playerAngle - angle;
  f32 wrappedDifference;
  if (difference > 0) {
    wrappedDifference = difference - 2 * M_PI;
  } else {
    wrappedDifference = difference + 2 * M_PI;
  }

  // Choose the direction based on the shortest arc
  if (fabs(difference) < fabs(wrappedDifference)) {
    if (difference < 0)
      hforce *= -1.0;
  } else {
    if (wrappedDifference < 0)
      hforce *= -1.0;
  }

  f32 hforceX = cos(hangle) * hforce;
  f32 hforceY = sin(hangle) * hforce;

  f32 forceX = vforceX + hforceX;
  f32 forceY = vforceY + hforceY;
  
  CBody* body = CmpGetBody(droneId);
  microPhysicsBodySetVelocity(body->body_id, forceX, forceY);
}

void DroneFree(int entityId)
{
  CBody *body = CmpGetBody(entityId);
  microPhysicsBodyFree(body->body_id);
}

void DroneAddEntity(const int x, const int y)
{
  int drone_eid = microECSEntityNew(NULL, DroneFree);
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
  const u32 animId = microAnimationGet("drone-1");
  CmpAddAnimation(drone_eid, animId, 0.1, FALSE, FALSE);
  CmpAddUpdate(drone_eid, DroneUpdate);

  const f32 bodyRadius = (DRONE_TEX_WIDTH / 2.0) * DRONE_SCALE / 2.0;
  int drone_bodyid = microPhysicsBodyNewCircle(drone_eid, 0, x, y, bodyRadius,
                                               0.01, FALSE, FALSE, 0, 0.0);
  microPhysicsBodySetCollisionCallback(drone_bodyid, DroneCollision);
  microPhysicsBodySetFilter(drone_bodyid, COLLISION_GROUP_ENEMY,
                            COLLISION_MASK_ENEMY);
  CmpAddBody(drone_eid, drone_bodyid);
  CmpAddHealth(drone_eid, 5, 5);

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(drone_eid, planetX, planetY);
}
