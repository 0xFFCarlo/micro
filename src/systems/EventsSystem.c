#include "EventsSystem.h"
#include "../components/LogicComponents.h"
#include "../micro/ECS.h"
#include "../util/debug.h"
#include <stdlib.h>

void eventsSystem(float dt)
{
  (void)(dt); // Unused parameter

  CEventListener *components_events = (CEventListener *)
    microECSComponentsGet(cid_event_listener);
  const unsigned int
    components_count = microECSComponentsCount(cid_event_listener);

  // Get the next event
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    for (uint32_t i = 0; i < components_count; i++)
    {
      const int entityId = microECSComponentGetEntityId(cid_event_listener, i);
      if (components_events[i].on_event)
        components_events[i].on_event(entityId, &event);
    }
  }
}
