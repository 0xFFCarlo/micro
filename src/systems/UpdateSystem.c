#include <stdlib.h>
#include "UpdateSystem.h"
#include "../components/CUpdate.h"
#include "../ECS.h"

int update_system_query = -1;

void updateSystem(float dt) {

  if (update_system_query == -1) {
    int components[1] = {cid_update};
    update_system_query = microECSQueryCreate(components, 1, NULL);
  }
  
  ecs_entity_list entities = microECSQueryRun(update_system_query);
  if (entities.size == 0) return;
  
  for (int i = 0; i < entities.size; i++) {
    const int entityId = entities.entityIds[i];
    CUpdate* update = (CUpdate*)microECSEntityGetComponent(entityId, cid_update);
    update->update(entityId, dt);
  }
}
