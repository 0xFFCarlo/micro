#include "Physics.h"

int microPhysicsWorldNew(MicroWorldCollisionsCb collisions_callback,
                         MicroWorldBodiesRemovedCb bodies_removed_callback)
{
  (void)collisions_callback;
  (void)bodies_removed_callback;
  return -1;
}

int microPhysicsWorldUseSpatialHash(int worldId, int cell_size, int cell_count)
{
  (void)worldId;
  (void)cell_size;
  (void)cell_count;
  return -1;
}

void microPhysicsWorldStep(int worldId, float dt)
{
  (void)worldId;
  (void)dt;
}

int microPhysicsWorldsCount() { return 0; }

int microPhysicsWorldGetBodyCount(int worldId)
{
  (void)worldId;
  return 0;
}

int microPhysicsWorldNewCollisionTilemap(int worldId,
                                         bool (*is_solid)(const int px,
                                                          const int py),
                                         int tile_size)
{
  (void)worldId;
  (void)is_solid;
  (void)tile_size;
  return -1;
}

void microPhysicsWorldFree(int worldId) { (void)worldId; }

void microPhysicsWorldFreeAll() {}

int microPhysicsBodyNewCircle(int entityId, int worldId, float cx, float cy,
                              float radius, float mass, uint8_t isStatic,
                              uint8_t canRotate, float elasticity,
                              float friction)
{
  (void)entityId;
  (void)worldId;
  (void)cx;
  (void)cy;
  (void)radius;
  (void)mass;
  (void)isStatic;
  (void)canRotate;
  (void)elasticity;
  (void)friction;
  return -1;
}

int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                            float width, float height, float mass,
                            uint8_t isStatic, uint8_t canRotate,
                            float elasticity, float friction)
{
  (void)entityId;
  (void)worldId;
  (void)cx;
  (void)cy;
  (void)width;
  (void)height;
  (void)mass;
  (void)isStatic;
  (void)canRotate;
  (void)elasticity;
  (void)friction;
  return -1;
}

void microPhysicsBodySetFilter(int bodyId, int category, uint32_t mask)
{
  (void)bodyId;
  (void)category;
  (void)mask;
}

void microPhysicsBodySetCollisionBeginCallback(int bodyId,
                                               void (*callback)(int, int))
{
  (void)bodyId;
  (void)callback;
}

void microPhysicsBodySetCollisionUpdateCallback(int bodyId,
                                                void (*callback)(int, int))
{
  (void)bodyId;
  (void)callback;
}

void microPhysicsBodySetCollisionTilemap(int bodyId, int tilemapId)
{
  (void)bodyId;
  (void)tilemapId;
}

void microPhysicsBodyFree(int bodyId) { (void)bodyId; }

void microPhysicsBodySetMass(int bodyId, float mass)
{
  (void)bodyId;
  (void)mass;
}

float microPhysicsBodyGetMass(int bodyId)
{
  (void)bodyId;
  return 0.0f;
}

void microPhysicsBodySetPosition(int bodyId, float x, float y)
{
  (void)bodyId;
  (void)x;
  (void)y;
}

void microPhysicsBodySetVelocity(int bodyId, float x, float y)
{
  (void)bodyId;
  (void)x;
  (void)y;
}

void microPhysicsBodySetForce(int bodyId, float x, float y)
{
  (void)bodyId;
  (void)x;
  (void)y;
}

void microPhysicsBodyApplyForce(int bodyId, float x, float y)
{
  (void)bodyId;
  (void)x;
  (void)y;
}

void microPhysicsBodyGetPosition(int bodyId, float *x, float *y)
{
  (void)bodyId;
  if (x)
    *x = 0.0f;
  if (y)
    *y = 0.0f;
}

void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y)
{
  (void)bodyId;
  if (x)
    *x = 0.0f;
  if (y)
    *y = 0.0f;
}

void microPhysicsBodyGetForce(int bodyId, float *x, float *y)
{
  (void)bodyId;
  if (x)
    *x = 0.0f;
  if (y)
    *y = 0.0f;
}

void microPhysicsBodySetRotation(int bodyId, float angle)
{
  (void)bodyId;
  (void)angle;
}

float microPhysicsBodyGetRotation(int bodyId)
{
  (void)bodyId;
  return 0.0f;
}

void microPhysicsBodySetSensor(int bodyId, bool is_sensor)
{
  (void)bodyId;
  (void)is_sensor;
}

bool microPhysicsBodyIsSensor(int bodyId)
{
  (void)bodyId;
  return false;
}

bool microPhysicsBodyIsStatic(int bodyId)
{
  (void)bodyId;
  return false;
}

bool microPhysicsBodyIsValid(int bodyId)
{
  (void)bodyId;
  return false;
}

int microPhysicsBodyGetWorldId(int bodyId)
{
  (void)bodyId;
  return -1;
}

MicroAABB microPhysicsBodyGetAABB(int bodyId)
{
  (void)bodyId;
  MicroAABB box = {0};
  return box;
}
