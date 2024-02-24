#include "PhysicsSystem.h"
#include "../components/MotionComponents.h"
#include "../core/ECS.h"
#include "../core/Physics.h"
#include <math.h>
#include <stdlib.h>

int world_id = 0;

void physicsSystem(float dt)
{
  CBody *components_body = microECSComponentsGet(cid_body);
  const unsigned int components_count = microECSComponentsCount(cid_body);

  // Update physics worlds and bodies
  int worlds_count = microPhysicsWorldsCount();
  for (int i = 0; i < worlds_count; i++)
    microPhysicsWorldStep(i, dt);

  // Update positions of all entities with a body component
  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_body, i);
    if (!microECSEntityIsAlive(entityId))
      continue;
    CPosition *pos = (CPosition *)microECSEntityGetComponent(entityId,
                                                             cid_position);
    CBody *body = &components_body[i];
    float x, y;
    microPhysicsBodyGetPosition(body->body_id, &x, &y);
    pos->x = x;
    pos->y = y;
  }
}
