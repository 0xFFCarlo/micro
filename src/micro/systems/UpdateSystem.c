#include "../components/LogicComponents.h"
#include "../core/ECS.h"
#include <stdlib.h>

static void update_system_update(float dt)
{
  CUpdate *components_update = (CUpdate *)microECSComponentsGet(cid_update);
  const unsigned int components_count = microECSComponentsCount(cid_update);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_update, i);
    if (microECSEntityIsAlive(entityId) == 0)
      continue;
    components_update[i].update(entityId, dt);
  }
}

MicroECSSystem update_system = {update_system_update, NULL, NULL, NULL};
