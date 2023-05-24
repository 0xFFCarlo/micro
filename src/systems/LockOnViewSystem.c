#include <stdlib.h>
#include <stdio.h>
#include "LockOnViewSystem.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../ECS.h"
#include "../Graphics.h"

int lock_on_view_system_query = -1;

void lockOnViewSystem(float dt)
{
  if (lock_on_view_system_query == -1) {
    int components[2] = {cid_position, cid_lock_on_view};
    lock_on_view_system_query = microECSQueryCreate(components, 2, NULL);
  } 

  ecs_entity_list entities = microECSQueryRun(lock_on_view_system_query);
  if (entities.size == 0) return;

  if (entities.size > 1)
    printf("Warning: more than one entity with lock on view component\n");
  
  for (int i = 0; i < entities.size; i++) {
    const int entityId = entities.entityIds[i];
    CPosition* position = (CPosition*)microECSEntityGetComponent(entityId, cid_position);
    CLockOnView* lockOnView = (CLockOnView*)microECSEntityGetComponent(entityId, cid_lock_on_view);
    
    microViewSetCenter(position->x, position->y);

    if (lockOnView->followRotation > 0) {
      CTransform* t = (CTransform*)microECSEntityGetComponent(entityId, cid_transform);
      microViewSetRotation(t->rotation);
    }
  }
}
