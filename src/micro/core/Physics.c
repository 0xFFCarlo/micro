#include "Physics.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include <chipmunk/chipmunk.h>
#include <limits.h>

#define COLLISION_MAPS_MAX 8
#define WORLD_ID_BIT_SHIFT 24
#define SHAPE_ID_BIT_MASK 0xFFFFFF

typedef struct CollisionTileMap
{
  int tile_size;
  bool (*is_solid)(int px, int py);
} CollisionTileMap;

typedef struct
{
  cpSpace *space;
  Vector shapes;
  Vector freed_shapes_id;
  int is_simulating;
  Vector to_add;
  Vector to_remove;
  CollisionTileMap collision_maps[COLLISION_MAPS_MAX];
} World;

typedef struct BodyData
{
  int entityId;
  void (*collisionBegin)(int e1, int e2);
  void (*collisionUpdate)(int e1, int e2);
  int tilemapId;
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

// Collision callback
cpBool microPhysicsCollisionUpdate(cpArbiter *arb, cpSpace *space,
                                   cpDataPointer userData)
{
  (void)space;    // Unused
  (void)userData; // Unused

  CP_ARBITER_GET_BODIES(arb, body1, body2);
  BodyData *data1 = (BodyData *)cpBodyGetUserData(body1);
  assert(data1 != NULL);
  BodyData *data2 = (BodyData *)cpBodyGetUserData(body2);
  assert(data2 != NULL);

  if (data1->collisionUpdate)
    data1->collisionUpdate(data1->entityId, data2->entityId);

  if (data2->collisionUpdate)
    data2->collisionUpdate(data2->entityId, data1->entityId);

  return cpTrue;
}

int microPhysicsWorldNew()
{
  if (!worlds_initialized)
  {
    worlds = vector_create(sizeof(World));
    worlds_initialized = 1;
  }

  World world = {
    .space = cpSpaceNew(),
    .shapes = vector_create(sizeof(cpShape *)),
    .freed_shapes_id = vector_create(sizeof(int)),
    .to_remove = vector_create(sizeof(int)),
    .to_add = vector_create(sizeof(int)),
  };
  cpSpaceSetGravity(world.space, cpv(0, 0));

  // Initialize collision maps
  for (int i = 0; i < COLLISION_MAPS_MAX; i++)
    world.collision_maps[i].is_solid = NULL;

  // Set collision handler
  cpCollisionHandler *handler = cpSpaceAddCollisionHandler(world.space, 0, 0);
  handler->beginFunc = microPhysicsCollisionBegin;
  handler->preSolveFunc = microPhysicsCollisionUpdate;

  vector_push_back(&worlds, &world);
  return worlds.size - 1;
}

int microPhysicsWorldUseSpatialHash(int worldId, int cell_size, int cell_count)
{
  World *world = vector_at(&worlds, worldId);
  cpSpaceUseSpatialHash(world->space, cell_size, cell_count);
  return 0;
}

static inline int divfloor(int n, int d)
{
  return n / d - ((n >> 31) & ((n % d) != 0));
}

void microPhysicsWorldStep(int worldId, float dt)
{
  World *world = vector_at(&worlds, worldId);

  // Remove queued shapes
  while (world->to_remove.size)
  {
    const int bodyId = *(int *)vector_back(&world->to_remove);
    vector_pop_back(&world->to_remove);
    microPhysicsBodyFree(bodyId);
  }

  // Add queued shapes
  while (world->to_add.size)
  {
    const int bodyId = *(int *)vector_back(&world->to_add);
    vector_pop_back(&world->to_add);
    const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
    cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
    cpSpaceAddBody(world->space, cpShapeGetBody(shape));
    cpSpaceAddShape(world->space, shape);
  }

  world->is_simulating = true;

  // Ierating over each body
  for (uint32_t i = 0; i < world->shapes.size; i++)
  {
    cpShape *shape = *(cpShape **)vector_at(&world->shapes, i);
    if (shape == NULL)
      continue;
    cpBody *body = cpShapeGetBody(shape);
    BodyData *data = cpBodyGetUserData(body);
    int tid = data->tilemapId;
    if (tid == -1)
      continue;
    CollisionTileMap *map = &world->collision_maps[data->tilemapId];
    const int ts = map->tile_size;
    cpVect pos = cpBodyGetPosition(body);
    cpVect vel = cpBodyGetVelocity(body);
    const int bwidth = (cpShapeGetBB(shape).r - cpShapeGetBB(shape).l) / 2.f;
    const int bheight = (cpShapeGetBB(shape).t - cpShapeGetBB(shape).b) / 2.f;

    // Resolve collision with tilemap
    if (vel.x < 0)
    {
      // check collision with left
      int nextTile_lt_x = divfloor(pos.x - bwidth + vel.x * dt, ts);
      int nextTile_lt_y = divfloor(pos.y - bheight + 1, ts);
      int nextTile_lb_x = divfloor(pos.x - bwidth + vel.x * dt, ts);
      int nextTile_lb_y = divfloor(pos.y + bheight - 1, ts);
      if (map->is_solid(nextTile_lt_x * ts, nextTile_lt_y * ts) ||
          map->is_solid(nextTile_lb_x * ts, nextTile_lb_y * ts))
      {
        vel.x = 0;
        pos.x = (nextTile_lt_x + 1) * ts + bwidth;
      }
    }

    if (vel.x > 0)
    {
      // check collision with right
      int nextTile_rt_x = divfloor(pos.x + bwidth + vel.x * dt, ts);
      int nextTile_rt_y = divfloor(pos.y - bheight + 1, ts);
      int nextTile_rb_x = divfloor(pos.x + bwidth + vel.x * dt, ts);
      int nextTile_rb_y = divfloor(pos.y + bheight - 1, ts);
      if (map->is_solid(nextTile_rt_x * ts, nextTile_rt_y * ts) ||
          map->is_solid(nextTile_rb_x * ts, nextTile_rb_y * ts))
      {
        vel.x = 0;
        pos.x = (nextTile_rt_x)*ts - bwidth;
      }
    }

    if (vel.y < 0)
    {
      // check collision with top
      int nextTile_tl_x = divfloor(pos.x - bwidth + 1, ts);
      int nextTile_tl_y = divfloor(pos.y - bheight + vel.y * dt, ts);
      int nextTile_tr_x = divfloor(pos.x + bwidth - 1, ts);
      int nextTile_tr_y = divfloor(pos.y - bheight + vel.y * dt, ts);
      if (map->is_solid(nextTile_tl_x * ts, nextTile_tl_y * ts) ||
          map->is_solid(nextTile_tr_x * ts, nextTile_tr_y * ts))
      {
        vel.y = 0;
        pos.y = (nextTile_tr_y + 1) * ts + bheight;
      }
    }

    if (vel.y > 0)
    {
      // check collision with bottom
      int nextTile_bl_x = divfloor(pos.x - bwidth + 1, ts);
      int nextTile_bl_y = divfloor(pos.y + bheight + vel.y * dt, ts);
      int nextTile_br_x = divfloor(pos.x + bwidth - 1, ts);
      int nextTile_br_y = divfloor(pos.y + bheight + vel.y * dt, ts);
      if (map->is_solid(nextTile_bl_x * ts, nextTile_bl_y * ts) ||
          map->is_solid(nextTile_br_x * ts, nextTile_br_y * ts))
      {
        vel.y = 0;
        pos.y = (nextTile_br_y)*ts - bheight;
      }
    }

    cpBodySetPosition(body, pos);
    cpBodySetVelocity(body, vel);
  }

  cpSpaceStep(world->space, dt);

  world->is_simulating = false;
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

int microPhysicsWorldNewCollisionTilemap(int worldId,
                                         bool (*is_solid)(const int px, const int py),
                                         int tile_size)
{
  World *world = vector_at(&worlds, worldId);

  // Find a free collision map
  int id = -1;
  for (int i = 0; i < COLLISION_MAPS_MAX; i++)
  {
    if (world->collision_maps[i].is_solid == NULL)
    {
      id = i;
      break;
    }
  }

  // No free collision map
  if (id == -1)
    return -1;

  // Set collision map
  CollisionTileMap *map = &world->collision_maps[id];
  map->is_solid = is_solid;
  map->tile_size = tile_size;

  return id;
}

void microPhysicsWorldFree(int worldId)
{
  World *world = vector_at(&worlds, worldId);
  if (worlds.data[worldId] == NULL)
    return;

  // Free all bodies user data
  for (uint32_t i = 0; i < world->shapes.size; i++)
    microPhysicsBodyFree(i);

  cpSpaceFree(world->space);
  vector_free(&world->shapes);
  vector_free(&world->freed_shapes_id);
  vector_free(&world->to_add);
  vector_free(&world->to_remove);
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
                              float radius, float mass, uint8_t isStatic,
                              uint8_t canRotate, float elasticity, float friction)
{
  assert(mass != 0.f);
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
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  BodyData *data = malloc(sizeof(BodyData));
  data->entityId = entityId;
  data->collisionBegin = NULL;
  data->collisionUpdate = NULL;
  data->tilemapId = -1;
  cpBodySetUserData(body, data);
  cpShape *shape = cpCircleShapeNew(body, radius, cpv(0, 0));
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, friction);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);
  cpShapeSetMass(shape, mass);

  // Store shape in shapes vector
  int bodyId = -1;
  if (world->freed_shapes_id.size)
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vector_back(&world->freed_shapes_id);
    vector_pop_back(&world->freed_shapes_id);
    cpShape **shapes = (cpShape **)world->shapes.data;
    shapes[shapeId] = shape;
    bodyId = shapeId + (worldId << WORLD_ID_BIT_SHIFT);
  }
  else
  {
    // Add a new shape id
    vector_push_back(&world->shapes, &shape);
    bodyId = world->shapes.size - 1 + (worldId << WORLD_ID_BIT_SHIFT);
  }

  if (world->is_simulating)
  {
    // Queue shape to add
    vector_push_back(&world->to_add, &bodyId);
  }
  else
  {
    cpSpaceAddBody(world->space, body);
    cpSpaceAddShape(world->space, shape);
  }

  return bodyId;
}

int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                            float width, float height, float mass, uint8_t isStatic,
                            uint8_t canRotate, float elasticity, float friction)
{
  assert(mass != 0.f);
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
  cpBodySetPosition(body, cpv(cx, cy));
  cpBodySetVelocity(body, cpv(0, 0));
  cpBodySetForce(body, cpv(0, 0));
  BodyData *data = malloc(sizeof(BodyData));
  data->entityId = entityId;
  data->collisionBegin = NULL;
  data->collisionUpdate = NULL;
  data->tilemapId = -1;
  cpBodySetUserData(body, data);
  cpShape *shape = cpBoxShapeNew(body, width, height, 0);
  cpShapeSetElasticity(shape, elasticity);
  cpShapeSetFriction(shape, friction);
  cpShapeSetCollisionType(shape, 0);
  cpShapeSetSensor(shape, 0);
  cpShapeSetUserData(shape, NULL);

  // Store shape in shapes vector
  int bodyId = -1;
  if (world->freed_shapes_id.size)
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vector_back(&world->freed_shapes_id);
    vector_pop_back(&world->freed_shapes_id);
    cpShape **shapes = (cpShape **)world->shapes.data;
    shapes[shapeId] = shape;
    bodyId = shapeId + (worldId << WORLD_ID_BIT_SHIFT);
  }
  else
  {
    // Add a new shape id
    vector_push_back(&world->shapes, &shape);
    bodyId = world->shapes.size - 1 + (worldId << WORLD_ID_BIT_SHIFT);
  }

  if (world->is_simulating)
  {
    // Queue shape to add
    vector_push_back(&world->to_add, &bodyId);
  }
  else
  {
    cpSpaceAddBody(world->space, body);
    cpSpaceAddShape(world->space, shape);
  }

  return bodyId;
}

void microPhysicsBodyFree(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  if (worlds.data[worldId] == NULL) // world does not exist
    return;
  World *world = vector_at(&worlds, worldId);
  int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  cpShape **shapes = (cpShape **)world->shapes.data;
  if (shapes[shapeId] == NULL) // shape already freed
    return;
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  if (shape == NULL)
    return;

  if (world->is_simulating)
  {
    // Queue shape to remove and disable collision
    vector_push_back(&world->to_remove, &bodyId);
  }
  else
  {
    // Free shape
    cpBody *body = cpShapeGetBody(shape);
    BodyData *data = cpBodyGetUserData(body);
    free(data);
    cpSpaceRemoveBody(world->space, body);
    cpSpaceRemoveShape(world->space, shape);
    cpBodyFree(body);
    cpShapeFree(shape);

    // Remove shape from shapes vector
    shapes[shapeId] = NULL;
    vector_push_back(&world->freed_shapes_id, &shapeId);
  }
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
  assert(mass != 0.f);
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  cpShapeSetMass(shape, mass);
}

float microPhysicsBodyGetMass(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  return cpShapeGetMass(shape);
}

void microPhysicsBodySetPosition(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetPosition(body, cpv(x, y));
}

void microPhysicsBodySetVelocity(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetVelocity(body, cpv(x, y));
}

void microPhysicsBodySetForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetForce(body, cpv(x, y));
}

void microPhysicsBodyApplyForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;

  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);

  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodyApplyForceAtLocalPoint(body, cpv(x, y), cpv(0, 0));
}

void microPhysicsBodySetCollisionBeginCallback(int bodyId,
                                               void (*callback)(int, int))
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  assert(body != NULL);
  BodyData *data = cpBodyGetUserData(body);
  data->collisionBegin = callback;
}

void microPhysicsBodySetCollisionUpdateCallback(int bodyId,
                                                void (*callback)(int, int))
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  assert(body != NULL);
  BodyData *data = cpBodyGetUserData(body);
  data->collisionUpdate = callback;
}

void microPhysicsBodySetFilter(int bodyId, int category, uint32_t mask)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  assert(category >= 0 && category <= INT_MAX);
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpShapeFilter filter = cpShapeFilterNew(CP_NO_GROUP, category, mask);
  cpShapeSetFilter(shape, filter);
}

void microPhysicsBodySetCollisionTilemap(int bodyId, int tilemapId)
{
  assert(tilemapId >= -1 && tilemapId < COLLISION_MAPS_MAX);
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  BodyData *data = cpBodyGetUserData(body);
  data->tilemapId = tilemapId;
}

void microPhysicsBodyGetPosition(int bodyId, float *x, float *y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
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
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
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
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
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
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
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
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  const cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  const cpBody *body = cpShapeGetBody(shape);
  return cpBodyGetAngle(body);
}

int microPhysicsBodyGetWorldId(int bodyId)
{
  return bodyId >> WORLD_ID_BIT_SHIFT;
}

void microPhysicsBodySetSensor(int bodyId, bool is_sensor)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpShapeSetSensor(shape, is_sensor);
}

bool microPhysicsBodyIsSensor(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  return cpShapeGetSensor(shape);
}

bool microPhysicsBodyIsStatic(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  return cpBodyGetType(body) == CP_BODY_TYPE_STATIC;
}

MicroAABB microPhysicsBodyGetAABB(int bodyId)
{
  MicroAABB aabb = {0};
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  cpShape *cp_shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  if (cp_shape == NULL)
    return aabb;
  cpBB bb = cpShapeGetBB(cp_shape);
  aabb.left = bb.l;
  aabb.bottom = bb.b;
  aabb.right = bb.r;
  aabb.top = bb.t;
  return aabb;
}

bool microPhysicsBodyIsValid(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = vector_at(&worlds, worldId);
  if (shapeId < 0 || shapeId >= (int)world->shapes.size)
    return false;
  cpShape *cp_shape = *(cpShape **)vector_at(&world->shapes, shapeId);
  if (cp_shape == NULL)
    return false;
  return true;
}
