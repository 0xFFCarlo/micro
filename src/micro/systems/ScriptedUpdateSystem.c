#include "../components/LogicComponents.h"
#include "../core/ECS.h"
#include <stdlib.h>

// callback
void (*scripted_update_callback)(int *eids, int count, float dt) = NULL;

static void scripted_update_system_update(float dt)
{
  const unsigned int
    components_count = microECSComponentsCount(cid_scripted_update);

  int *entities = malloc(sizeof(int) * components_count);
  for (unsigned int i = 0; i < components_count; i++)
    entities[i] = microECSComponentGetEntityId(cid_scripted_update, i);

  if (scripted_update_callback != NULL)
    scripted_update_callback(entities, components_count, dt);

  free(entities);
}

MicroECSSystem scripted_update_system = {scripted_update_system_update, NULL,
                                         NULL, NULL};
