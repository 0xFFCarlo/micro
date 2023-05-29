#include <stdlib.h>
#include "EventsSystem.h"
#include "../components/LogicComponents.h"
#include "../ECS.h"

void eventsSystem(float dt) {
  
  CEventListener* components_events = (CEventListener*)microECSComponentsGet(cid_event_listener);
  const unsigned int components_count = microECSComponentsCount(cid_event_listener);

  // Get the next event
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    for (int i = 0; i < components_count; i++) {
      const int entityId = microECSComponentGetEntityId(cid_event_listener, i);
      components_events[i].on_event(entityId, event);
    }
  }
}
