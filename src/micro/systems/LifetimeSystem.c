#include "../components/LogicComponents.h"
#include "../core/ECS.h"
#include "EventsSystem.h"
#include <stdlib.h>

void lifetimeSystem(float dt)
{
  CLifetime *components_lifetime = (CLifetime *)
    microECSComponentsGet(cid_lifetime);
  const unsigned int components_count = microECSComponentsCount(cid_lifetime);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_lifetime, i);
    CLifetime *lifetime = &components_lifetime[i];

    lifetime->lifetime -= dt;
    if (lifetime->lifetime <= 0)
      microECSEntityRemove(entityId);
  }
}
