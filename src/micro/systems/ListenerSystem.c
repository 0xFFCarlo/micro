#include "../components/AudioComponents.h"
#include "../components/MotionComponents.h"
#include "../core/ECS.h"
#include "../core/Audio.h"
#include "../util/debug.h"
#include <stdlib.h>
#include <assert.h>

static void listener_system_update(float dt)
{
  (void)dt;

  CListener *listeners = (CListener *)microECSComponentsGet(cid_listener);
  const unsigned int components_count = microECSComponentsCount(cid_listener);

  // There can be only one listener
  assert(components_count <= 1);

  if (components_count == 0)
    return;

  const int entityId = microECSComponentGetEntityId(cid_listener, 0);
  CListener *listener = &listeners[0];
  CPosition *position = CmpGetPosition(entityId);
  microListenerSetPosition(position->x, position->y);
  microListenerSetMaxDistance(listener->max_distance);
}

MicroECSSystem listener_system = {listener_system_update, NULL, NULL, NULL};
