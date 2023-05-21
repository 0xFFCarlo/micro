#include "VelocitySystem.h"
#include "../components/CPosition.h"
#include "../components/CVelocity.h"
#include "../ECS.h"
#include <stdlib.h>

int velocity_system_query = -1;

void velocitySystem(float dt)
{
  if (velocity_system_query == -1) {
    int components[2] = {cid_position, cid_velocity};
    velocity_system_query = microECSQueryCreate(components, 2, NULL);
  }

  ecs_entity_list entities = microECSQueryRun(velocity_system_query);
  if (entities.size == 0) return;

  for (int i = 0; i < entities.size; i++) {
    const int entityId = entities.entityIds[i];
    CPosition* position = (CPosition*)microECSEntityGetComponent(entityId, cid_position);
    CVelocity* velocity = (CVelocity*)microECSEntityGetComponent(entityId, cid_velocity);

    position->x += velocity->x * dt;
    position->y += velocity->y * dt;
  }
}
