#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include <stdlib.h>

static void light_system_update(float dt)
{
  (void)(dt); // Unused parameter

  CLightSource *components_light = (CLightSource *)
    microECSComponentsGet(cid_light_source);
  const int components_count = microECSComponentsCount(cid_light_source);

  // Update light positions
  for (int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_light_source, i);
    if (microECSEntityIsAlive(entityId) == 0)
      continue;

    CPosition *position = CmpGetPosition(entityId);
    CLightSource *light = &components_light[i];
    microLightSetPosition(light->lightId, position->x, position->y);
  }

  // Recalculate lights texture
  microLightsUpdateTexture();
}
 
MicroECSSystem light_system = {light_system_update, NULL, NULL, NULL};
