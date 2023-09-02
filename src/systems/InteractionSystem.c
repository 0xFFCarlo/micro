#include "InteractionSystem.h"
#include "../components/CustomComponents.h"
#include "../components/MotionComponents.h"
#include "../micro/ECS.h"
#include <math.h>
#include <stdlib.h>

int32_t interaction_actor_id = -1;

// Object that it is in range of the player for interaction
float object_dist = 0;
int32_t object_id = -1;
CInteractive *object_component = NULL;

void interactionSystemSetActorId(uint32_t entity_id)
{
  interaction_actor_id = entity_id;
}

int interactionSystemGetObjectId()
{
  return object_id;
}

uint8_t interactionSystemInteract()
{
  if (object_id == -1)
    return 0;

  if (object_component->on_interact == NULL)
    return 0;

  object_component->on_interact(object_id);
  return 1;
}

void interactionSystem(float dt)
{
  (void)(dt); // Unused parameter

  if (interaction_actor_id == -1)
    return;

  CPosition *player_pos = (CPosition *)
    microECSEntityGetComponent(interaction_actor_id, cid_position);

  CInteractive *components_interactive = (CInteractive *)
    microECSComponentsGet(cid_interactive);
  const unsigned int
    components_count = microECSComponentsCount(cid_interactive);

  // Find object that it is in range of the player for interaction
  object_dist = 16000.0;
  object_id = -1;
  object_component = NULL;
  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_interactive, i);
    CPosition *pos = (CPosition *)microECSEntityGetComponent(entityId,
                                                             cid_position);
    CInteractive *interactive = &components_interactive[i];

    if (interactive->on_interact == NULL || interactive->can_interact == 0)
      continue;

    float dist = sqrtf(powf(player_pos->x - pos->x, 2) +
                       powf(player_pos->y - pos->y, 2));

    if (dist < interactive->interact_range && dist < object_dist)
    {
      object_dist = dist;
      object_id = entityId;
      object_component = &components_interactive[i];
    }
  }
}
