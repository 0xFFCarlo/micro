#include "Physics.h"
#include <stdlib.h>
#include <memory.h>
#include <chipmunk/chipmunk.h>
#include "Vector.h"


typedef struct {
  cpSpace* space;
  Vector shapes;
} World;

Vector worlds;
int worlds_initialized = 0;

int microPhysicsWorldNew()
{
  if (!worlds_initialized) {
    worlds = vector_create(sizeof(World));
    worlds_initialized = 1;
  }

  World world ={
    .space = cpSpaceNew(),
    .shapes = vector_create(sizeof(cpShape*))
  };
  cpSpaceSetGravity(world.space, cpv(0, 0));
  vector_push_back(&worlds, &world);
  return worlds.size - 1;
}

void microPhysicsWorldStep(int worldId, float dt)
{
  World* world = &((World*)worlds.data)[worldId];
  cpSpaceStep(world->space, dt);
}

int microPhysicsWorldsCount()
{
  return worlds.size;
}

int microPhysicsWorldGetBodyCount(int worldId)
{
  World* world = &((World*)worlds.data)[worldId];
  return world->shapes.size;
}

void microPhysicsWorldFree(int worldId)
{
  World* world = &((World*)worlds.data)[worldId];
  cpSpaceFree(world->space);
  vector_free(&world->shapes);
  World* lastWorld = vector_back(&worlds);
  memcpy(world, &lastWorld, sizeof(World));
  vector_pop_back(&worlds);
}

int microPhysicsBodyNewCircle(int worldId, float cx, float cy, float radius, float mass, unsigned char isStatic, float moment, float elasticity, float friction)
{
  World* world = &((World*)worlds.data)[worldId];
  cpBody* body = NULL;
  if (isStatic)
    body = cpBodyNewStatic();
  else
    body = cpBodyNew(mass, cpMomentForCircle(mass, 0, radius, cpv(0, 0)));
  cpSpaceAddBody(world->space, body);
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  cpBodySetMoment(body, moment);
  cpShape* shape = cpSpaceAddShape(world->space, cpCircleShapeNew(body, radius, cpv(0, 0)));
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, 0.0f);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);
  vector_push_back(&world->shapes, &shape);
  return world->shapes.size - 1;
}

int microPhysicsBodyNewRect(int worldId, float cx, float cy, float width, float height, float mass, unsigned char isStatic, float elasticity)
{
  World* world = &((World*)worlds.data)[worldId];
  cpBody* body = NULL;
  if (isStatic)
    body = cpBodyNewStatic();
  else
    body = cpBodyNew(mass, cpMomentForBox(mass, width, height));
  cpSpaceAddBody(world->space, body);
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  cpShape* shape = cpSpaceAddShape(world->space, cpBoxShapeNew(body, width, height, fmin(width, height)));
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, 0.0f);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);
  vector_push_back(&world->shapes, &shape);
  return world->shapes.size - 1 + (worldId << 16);
}

void microPhysicsBodyFree(int bodyId)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpSpaceRemoveShape(world->space, shape);
  cpSpaceRemoveBody(world->space, body);
  cpShapeFree(shape);
  cpBodyFree(body);
  cpShape* lastShape = vector_back(&world->shapes);
  memcpy(shape, &lastShape, sizeof(void*));
  vector_pop_back(&world->shapes); 
}

int microPhysicsBodiesCount()
{
  int count = 0;
  for (int i = 0; i < worlds.size; i++) {
    World* world = &((World*)worlds.data)[i];
    count += world->shapes.size;
  }
  return count;
}

void microPhysicsBodySetPosition(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpBodySetPosition(body, cpv(x, y));
}

void microPhysicsBodySetVelocity(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpBodySetVelocity(body, cpv(x, y));
}

void microPhysicsBodySetForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpBodySetForce(body, cpv(x, y));
}

void microPhysicsBodyGetPosition(int bodyId, float* x, float* y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpVect pos = cpBodyGetPosition(body);
  *x = pos.x;
  *y = pos.y;
}

void microPhysicsBodyGetVelocity(int bodyId, float* x, float* y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpVect vel = cpBodyGetVelocity(body);
  *x = vel.x;
  *y = vel.y;
}

void microPhysicsBodyGetForce(int bodyId, float* x, float* y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World* world = &((World*)worlds.data)[worldId];
  cpShape* shape = world->shapes.data[shapeId];
  cpBody* body = cpShapeGetBody(shape);
  cpVect force = cpBodyGetForce(body);
  *x = force.x;
  *y = force.y;
}
