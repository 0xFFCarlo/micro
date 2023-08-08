#include "UpdateSystem.h"
#include "../components/LogicComponents.h"
#include "../micro/ECS.h"
#include <stdlib.h>

void updateSystem(float dt)
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
