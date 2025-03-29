#include "../components/MotionComponents.h"
#include "../core/ECS.h"
#include "../core/Physics.h"
#include "../util/debug.h"

static void interaction_system_update(float dt)
{
  (void)dt;

  CInteractable *components_body = microECSComponentsGet(cid_interactable);
  const unsigned int
    components_count = microECSComponentsCount(cid_interactable);

  // Update positions of all entities with a body component
  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_interactable, i);
    if (!microECSEntityIsAlive(entityId))
      continue;
    CPosition *pos = (CPosition *)microECSEntityGetComponent(entityId,
                                                             cid_position);
    CInteractable *intr = &components_body[i];
    float x, y;
    microPhysicsBodyGetPosition(intr->_sensor_body_id, &x, &y);
    if (microPhysicsBodyIsStatic(intr->_sensor_body_id))
    {
      assert((int)(pos->x + intr->_offsetX) == (int)x &&
             (int)(pos->y + intr->_offsetY) == (int)y);
    }
    else
    {
      microPhysicsBodySetPosition(intr->_sensor_body_id,
                                  pos->x + intr->_offsetX,
                                  pos->y + intr->_offsetY);
    }
  }
}

MicroECSSystem interaction_system = {interaction_system_update, NULL, NULL};
