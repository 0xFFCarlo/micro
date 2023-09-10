#include "Physics.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include <chipmunk/chipmunk.h>

typedef struct
{
  cpSpace *space;
  Vector shapes;
  Vector freed_shapes_id;
} World;

typedef struct BodyData
{
  int entityId;
  void (*collisionBegin)(int e1, int e2);
} BodyData;

Vector worlds;
int worlds_initialized = 0;

// Collision callback
cpBool microPhysicsCollisionBegin(cpArbiter *arb, cpSpace *space,
                                  cpDataPointer userData)
{
  (void)space;    // Unused
  (void)userData; // Unused

  CP_ARBITER_GET_BODIES(arb, body1, body2);
  BodyData *data1 = (BodyData *)cpBodyGetUserData(body1);
  assert(data1 != NULL);
  BodyData *data2 = (BodyData *)cpBodyGetUserData(body2);
  assert(data2 != NULL);

  if (data1->collisionBegin)
    data1->collisionBegin(data1->entityId, data2->entityId);

  if (data2->collisionBegin)
    data2->collisionBegin(data2->entityId, data1->entityId);

  return cpTrue;
}

int microPhysicsWorldNew()
{
  if (!worlds_initialized)
  {
    worlds = vector_create(sizeof(World));
    worlds_initialized = 1;
  }

  World world = {.space = cpSpaceNew(),
                 .shapes = vector_create(sizeof(cpShape *)),
                 .freed_shapes_id = vector_create(sizeof(int))};
  cpSpaceSetGravity(world.space, cpv(0, 0));

  // Set collision handler
  cpCollisionHandler *handler = cpSpaceAddCollisionHandler(world.space, 0, 0);
  handler->beginFunc = microPhysicsCollisionBegin;

  vector_push_back(&worlds, &world);
  return worlds.size - 1;
}

void microPhysicsWorldStep(int worldId, float dt)
{
  World *world = vector_at(&worlds, worldId);
  cpSpaceStep(world->space, dt);
}

int microPhysicsWorldsCount()
{
  return worlds.size;
}

int microPhysicsWorldGetBodyCount(int worldId)
{
  World *world = vector_at(&worlds, worldId);
  return world->shapes.size;
}

void microPhysicsWorldFree(int worldId)
{
  World *world = vector_at(&worlds, worldId);

  // Free all bodies user data
  for (uint32_t i = 0; i < world->shapes.size; i++)
    microPhysicsBodyFree(i);

  cpSpaceFree(world->space);
  vector_free(&world->shapes);
  vector_free(&world->freed_shapes_id);
  World **ws = (World **)worlds.data;
  ws[worldId] = NULL;
}

void microPhysicsWorldFreeAll()
{
  for (uint32_t i = 0; i < worlds.size; i++)
    microPhysicsWorldFree(i);
  vector_free(&worlds);
  worlds_initialized = 0;
}

int microPhysicsBodyNewCircle(int entityId, int worldId, float cx, float cy,
                              float radius, float mass, unsigned char isStatic,
                              unsigned char canRotate, float elasticity,
                              float friction)
{
  World *world = vector_at(&worlds, worldId);
  cpBody *body = NULL;
  if (isStatic)
  {
    body = cpBodyNewStatic();
  }
  else
  {
    float moment = INFINITY;
    if (canRotate)
      moment = cpMomentForCircle(mass, 0, radius, cpv(0, 0));
    body = cpBodyNew(mass, moment);
  }
  cpSpaceAddBody(world->space, body);
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  BodyData *data = malloc(sizeof(BodyData));
  data->entityId = entityId;
  data->collisionBegin = NULL;
  cpBodySetUserData(body, data);
  cpShape *shape = cpCircleShapeNew(body, radius, cpv(0, 0));
  cpSpaceAddShape(world->space, shape);
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, friction);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);
  cpShapeSetMass(shape, mass);

  // Store shape in shapes vector
  if (world->freed_shapes_id.size)
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vector_back(&world->freed_shapes_id);
    vector_pop_back(&world->freed_shapes_id);
    cpShape **shapes = (cpShape **)world->shapes.data;
    shapes[shapeId] = shape;
    return shapeId + (worldId << 16);
  }
  else
  {
    // Add a new shape id
    vector_push_back(&world->shapes, &shape);
    return world->shapes.size - 1 + (worldId << 16);
  }
}

int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                            float width, float height, float mass,
                            unsigned char isStatic, unsigned char canRotate,
                            float elasticity, float friction)
{
  World *world = vector_at(&worlds, worldId);
  cpBody *body = NULL;
  if (isStatic)
  {
    body = cpBodyNewStatic();
  }
  else
  {
    float moment = INFINITY;
    if (canRotate)
      moment = cpMomentForBox(mass, width, height);
    body = cpBodyNew(mass, moment);
  }
  cpSpaceAddBody(world->space, body);
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  BodyData *data = malloc(sizeof(BodyData));
  data->entityId = entityId;
  data->collisionBegin = NULL;
  cpBodySetUserData(body, data);
  cpShape *shape = cpBoxShapeNew(body, width, height, 0);
  cpSpaceAddShape(world->space, shape);
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, friction);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);

  // Store shape in shapes vector
  if (world->freed_shapes_id.size)
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vector_back(&world->freed_shapes_id);
    vector_pop_back(&world->freed_shapes_id);
    cpShape **shapes = (cpShape **)world->shapes.data;
    shapes[shapeId] = shape;
    return shapeId + (worldId << 16);
  }
  else
  {
    // Add a new shape id
    vector_push_back(&world->shapes, &shape);
    return world->shapes.size - 1 + (worldId << 16);
  }
}

void microPhysicsBodyFree(int bodyId)
{
  const int worldId = bodyId >> 16;
  int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  if (shape == NULL)
    return;
  cpBody *body = cpShapeGetBody(shape);
  BodyData *data = cpBodyGetUserData(body);
  free(data);
  cpSpaceRemoveBody(world->space, body);
  cpSpaceRemoveShape(world->space, shape);
  cpBodyFree(body);
  cpShapeFree(shape);

  // Remove shape from shapes vector
  cpShape **shapes = (cpShape **)world->shapes.data;
  shapes[shapeId] = NULL;
  vector_push_back(&world->freed_shapes_id, &shapeId);
}

int microPhysicsBodiesCount()
{
  int count = 0;
  for (unsigned int i = 0; i < worlds.size; i++)
  {
    World *world = vector_at(&worlds, i);
    count += world->shapes.size - world->freed_shapes_id.size;
  }
  return count;
}

void microPhysicsBodySetMass(int bodyId, float mass)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  cpShapeSetMass(shape, mass);
}

float microPhysicsBodyGetMass(int bodyId)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  return cpShapeGetMass(shape);
}

void microPhysicsBodySetPosition(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetPosition(body, cpv(x, y));
}

void microPhysicsBodySetVelocity(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetVelocity(body, cpv(x, y));
}

void microPhysicsBodySetForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetForce(body, cpv(x, y));
}

void microPhysicsBodySetCollisionCallback(int bodyId,
                                          void (*callback)(int, int))
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  assert(body != NULL);
  BodyData *data = cpBodyGetUserData(body);
  data->collisionBegin = callback;
}

void microPhysicsBodySetFilter(int bodyId, int category, uint32_t mask)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  assert(category >= 0 && category <= 31);
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpShapeFilter filter = cpShapeFilterNew(CP_NO_GROUP, category, mask);
  cpShapeSetFilter(shape, filter);
}

void microPhysicsBodyGetPosition(int bodyId, float *x, float *y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpVect pos = cpBodyGetPosition(body);
  *x = pos.x;
  *y = pos.y;
}

void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpVect vel = cpBodyGetVelocity(body);
  *x = vel.x;
  *y = vel.y;
}

void microPhysicsBodyGetForce(int bodyId, float *x, float *y)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpVect force = cpBodyGetForce(body);
  *x = force.x;
  *y = force.y;
}

void microPhysicsBodySetRotation(int bodyId, float angle)
{
  const int worldId = bodyId >> 16;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetAngle(body, angle);
  // cpSpaceReindexShapesForBody(world->space, body);
  //  cpBodySetAngularVelocity(body, 0);
  //  cpBodySetTorque(body, 0);
}

float microPhysicsBodyGetRotation(int bodyId)
{
  const int worldId = bodyId >> 1;
  const int shapeId = bodyId & 0xFFFF;
  World *world = vector_at(&worlds, worldId);
  const cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  const cpBody *body = cpShapeGetBody(shape);
  return cpBodyGetAngle(body);
}
