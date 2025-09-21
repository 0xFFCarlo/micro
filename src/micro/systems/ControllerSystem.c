#include "../components/LogicComponents.h"
#include "../core/ECS.h"
#include "../util/debug.h"

static void controller_system_update(float dt)
{
  (void)(dt); // Unused parameter

  _CController *comps = (_CController *)microECSComponentsGet(cid_controller);
  const unsigned int components_count = microECSComponentsCount(cid_controller);

  uint64_t current_controller_uid = 0;
  uint32_t current_controller_offset = 0;
  for (unsigned int i = 0; i < components_count; i++)
  {
    if (comps[i].uid > current_controller_uid)
    {
      current_controller_uid = comps[i].uid;
      current_controller_offset = i;
    }
    comps[i].has_control = false;
  }
  if (components_count > 0)
    comps[current_controller_offset].has_control = true;
}

MicroECSSystem controller_system = {controller_system_update, NULL, NULL, NULL};
