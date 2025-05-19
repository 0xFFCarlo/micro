#include "../core/ECS.h"
#include "Systems.h"

int microSystemsUseAll()
{
  microECSSystemAdd(animation_system);
  microECSSystemAdd(events_system);
  microECSSystemAdd(follow_system);
  microECSSystemAdd(interaction_system);
  microECSSystemAdd(lifetime_system);
  microECSSystemAdd(light_system);
  microECSSystemAdd(listener_system);
  microECSSystemAdd(lock_on_view_system);
  microECSSystemAdd(particles_system);
  microECSSystemAdd(physics_system);
  microECSSystemAdd(tilemap_system);
  microECSSystemAdd(rendering_system);
  microECSSystemAdd(shaded_canvas_system);
  microECSSystemAdd(update_system);
  return 0;
}
