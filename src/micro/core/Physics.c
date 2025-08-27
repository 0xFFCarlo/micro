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
  cpShape **shapes;
  int *freed_shapes_id;
  int is_simulating;
  int *to_add;
  int *to_remove;
  MicroCollisionPair *collision_begins;
  MicroCollisionPair *collision_updates;
  MicroWorldCollisionsCb collision_callback;
  MicroWorldBodiesRemovedCb bodies_removed_callback;
  CollisionTileMap collision_maps[COLLISION_MAPS_MAX];
} World;

typedef struct BodyData
{
  int entityId;
  void (*collisionBegin)(int e1, int e2);
  void (*collisionUpdate)(int e1, int e2);
  int tilemapId;
} BodyData;

World *worlds = NULL;

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

  // Store collision pair
  const MicroCollisionPair pair = {data1->entityId, data2->entityId};
  const int worldId = data1->entityId >> WORLD_ID_BIT_SHIFT;
  World *world = &worlds[worldId];
  vec_append(world->collision_begins, &pair);

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

  // Store collision pair
  const MicroCollisionPair pair = {data1->entityId, data2->entityId};
  const int worldId = data1->entityId >> WORLD_ID_BIT_SHIFT;
  World *world = &worlds[worldId];
  vec_append(world->collision_updates, &pair);

  if (data1->collisionUpdate)
    data1->collisionUpdate(data1->entityId, data2->entityId);

  if (data2->collisionUpdate)
    data2->collisionUpdate(data2->entityId, data1->entityId);

  return cpTrue;
}

int microPhysicsWorldNew(MicroWorldCollisionsCb collisions_callback,
                         MicroWorldBodiesRemovedCb bodies_removed_callback)
{
  if (!worlds)
    worlds = vec_new(sizeof(World));

  World world = {
    .space = cpSpaceNew(),
    .shapes = vec_new(sizeof(cpShape *)),
    .freed_shapes_id = vec_new(sizeof(int)),
    .to_remove = vec_new(sizeof(int)),
    .to_add = vec_new(sizeof(int)),
    .collision_begins = vec_new(sizeof(MicroCollisionPair)),
    .collision_updates = vec_new(sizeof(MicroCollisionPair)),
    .collision_callback = collisions_callback,
    .bodies_removed_callback = bodies_removed_callback,
  };
  cpSpaceSetGravity(world.space, cpv(0, 0));

  // Initialize collision maps
  for (int i = 0; i < COLLISION_MAPS_MAX; i++)
    world.collision_maps[i].is_solid = NULL;

  // Set collision handler
  cpCollisionHandler *handler = cpSpaceAddCollisionHandler(world.space, 0, 0);
  handler->beginFunc = microPhysicsCollisionBegin;
  handler->preSolveFunc = microPhysicsCollisionUpdate;

  vec_append(worlds, &world);
  return vec_len(worlds) - 1;
}

int microPhysicsWorldUseSpatialHash(int worldId, int cell_size, int cell_count)
{
  World *world = &worlds[worldId];
  cpSpaceUseSpatialHash(world->space, cell_size, cell_count);
  return 0;
}

static inline int divfloor(int n, int d)
{
  return n / d - ((n >> 31) & ((n % d) != 0));
}

void microPhysicsWorldStep(int worldId, float dt)
{
  World *world = &worlds[worldId];

  // Clear collision pairs
  vec_clear(world->collision_begins);
  vec_clear(world->collision_updates);

  if (world->bodies_removed_callback && vec_len(world->to_remove))
  {
    // Call bodies removed callback
    world->bodies_removed_callback((int *)world->to_remove,
                                   vec_len(world->to_remove));
  }

  // Remove queued shapes
  while (vec_len(world->to_remove) > 0)
  {
    const int bodyId = *(int *)vec_back(world->to_remove);
    vec_pop_back(world->to_remove);
    microPhysicsBodyFree(bodyId);
  }

  // Add queued shapes
  while (vec_len(world->to_add) > 0)
  {
    const int bodyId = *(int *)vec_back(world->to_add);
    vec_pop_back(world->to_add);
    const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
    cpShape *shape = world->shapes[shapeId];
    cpSpaceAddBody(world->space, cpShapeGetBody(shape));
    cpSpaceAddShape(world->space, shape);
  }

  world->is_simulating = true;

  // Ierating over each body
  for (uint32_t i = 0; i < vec_len(world->shapes); i++)
  {
    cpShape *shape = world->shapes[i];
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

  if (world->collision_callback &&
      (vec_len(world->collision_begins) || vec_len(world->collision_updates)))
  {
    // Call collision callback
    world->collision_callback((MicroCollisionPair *)world->collision_begins,
                              vec_len(world->collision_begins),
                              (MicroCollisionPair *)world->collision_updates,
                              vec_len(world->collision_updates));
  }
}

int microPhysicsWorldsCount()
{
  return vec_len(worlds);
}

int microPhysicsWorldGetBodyCount(int worldId)
{
  World *world = &worlds[worldId];
  return vec_len(world->shapes) - vec_len(world->freed_shapes_id);
}

int microPhysicsWorldNewCollisionTilemap(int worldId,
                                         bool (*is_solid)(const int px,
                                                          const int py),
                                         int tile_size)
{
  World *world = &worlds[worldId];

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
  World *world = &worlds[worldId];

  // Free all bodies user data
  for (int32_t i = vec_len(world->shapes); i >= 0; i--)
    microPhysicsBodyFree(i);

  cpSpaceFree(world->space);
  vec_free(world->shapes);
  vec_free(world->freed_shapes_id);
  vec_free(world->to_add);
  vec_free(world->to_remove);
  vec_free(world->collision_begins);
  vec_free(world->collision_updates);
}

void microPhysicsWorldFreeAll()
{
  for (uint32_t i = 0; i < vec_len(worlds); i++)
    microPhysicsWorldFree(i);
  vec_free(worlds);
}

int microPhysicsBodyNewCircle(int entityId, int worldId, float cx, float cy,
                              float radius, float mass, uint8_t isStatic,
                              uint8_t canRotate, float elasticity,
                              float friction)
{
  assert(mass != 0.f);
  World *world = &worlds[worldId];
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
  if (vec_len(world->freed_shapes_id))
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vec_back(world->freed_shapes_id);
    vec_pop_back(world->freed_shapes_id);
    world->shapes[shapeId] = shape;
    bodyId = shapeId + (worldId << WORLD_ID_BIT_SHIFT);
  }
  else
  {
    // Add a new shape id
    vec_append(world->shapes, &shape);
    bodyId = vec_len(world->shapes) - 1 + (worldId << WORLD_ID_BIT_SHIFT);
  }

  if (world->is_simulating)
  {
    // Queue shape to add
    vec_append(world->to_add, &bodyId);
  }
  else
  {
    cpSpaceAddBody(world->space, body);
    cpSpaceAddShape(world->space, shape);
  }

  return bodyId;
}

int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                            float width, float height, float mass,
                            uint8_t isStatic, uint8_t canRotate,
                            float elasticity, float friction)
{
  assert(mass != 0.f);
  World *world = &worlds[worldId];
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
  if (vec_len(world->freed_shapes_id) > 0)
  {
    // Reuse a freed shape id
    int shapeId = *(int *)vec_back(world->freed_shapes_id);
    vec_pop_back(world->freed_shapes_id);
    world->shapes[shapeId] = shape;
    bodyId = shapeId + (worldId << WORLD_ID_BIT_SHIFT);
  }
  else
  {
    // Add a new shape id
    vec_append(world->shapes, &shape);
    bodyId = vec_len(world->shapes) - 1 + (worldId << WORLD_ID_BIT_SHIFT);
  }

  if (world->is_simulating)
  {
    // Queue shape to add
    vec_append(world->to_add, &bodyId);
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
  World *world = &worlds[worldId];
  int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  cpShape *shape = world->shapes[shapeId];
  if (shape == NULL)
    return; // Already freed

  if (world->is_simulating)
  {
    // Queue shape to remove and disable collision
    vec_append(world->to_remove, &bodyId);
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
    world->shapes[shapeId] = NULL;
    vec_append(world->freed_shapes_id, &shapeId);
  }
}

int microPhysicsBodiesCount()
{
  int count = 0;
  for (unsigned int i = 0; i < vec_len(worlds); i++)
  {
    World *world = &worlds[i];
    count += vec_len(world->shapes) - vec_len(world->freed_shapes_id);
  }
  return count;
}

void microPhysicsBodySetMass(int bodyId, float mass)
{
  assert(mass != 0.f);
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  cpShapeSetMass(shape, mass);
}

float microPhysicsBodyGetMass(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  return cpShapeGetMass(shape);
}

void microPhysicsBodySetPosition(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetPosition(body, cpv(x, y));
}

void microPhysicsBodySetVelocity(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetVelocity(body, cpv(x, y));
}

void microPhysicsBodySetForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodySetForce(body, cpv(x, y));
}

void microPhysicsBodyApplyForce(int bodyId, float x, float y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;

  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];

  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  cpBodyApplyForceAtLocalPoint(body, cpv(x, y), cpv(0, 0));
}

void microPhysicsBodySetCollisionBeginCallback(int bodyId,
                                               void (*callback)(int, int))
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpShapeFilter filter = cpShapeFilterNew(CP_NO_GROUP, category, mask);
  cpShapeSetFilter(shape, filter);
}

void microPhysicsBodySetCollisionTilemap(int bodyId, int tilemapId)
{
  assert(tilemapId >= -1 && tilemapId < COLLISION_MAPS_MAX);
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  BodyData *data = cpBodyGetUserData(body);
  data->tilemapId = tilemapId;
}

void microPhysicsBodyGetPosition(int bodyId, float *x, float *y)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  const cpShape *shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpShapeSetSensor(shape, is_sensor);
}

bool microPhysicsBodyIsSensor(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  return cpShapeGetSensor(shape);
}

bool microPhysicsBodyIsStatic(int bodyId)
{
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *shape = world->shapes[shapeId];
  assert(shape != NULL);
  cpBody *body = cpShapeGetBody(shape);
  return cpBodyGetType(body) == CP_BODY_TYPE_STATIC;
}

MicroAABB microPhysicsBodyGetAABB(int bodyId)
{
  MicroAABB aabb = {0};
  const int worldId = bodyId >> WORLD_ID_BIT_SHIFT;
  const int shapeId = bodyId & SHAPE_ID_BIT_MASK;
  World *world = &worlds[worldId];
  cpShape *cp_shape = world->shapes[shapeId];
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
  World *world = &worlds[worldId];
  if (shapeId < 0 || shapeId >= (int)vec_len(world->shapes))
    return false;
  cpShape *cp_shape = world->shapes[shapeId];
  if (cp_shape == NULL)
    return false;
  return true;
}
