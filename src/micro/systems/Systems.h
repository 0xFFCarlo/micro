#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "../core/ECS.h"

extern const MicroECSSystem animation_system;
extern const MicroECSSystem events_system;
extern const MicroECSSystem follow_system;
extern const MicroECSSystem interaction_system;
extern const MicroECSSystem lifetime_system;
extern const MicroECSSystem light_system;
extern const MicroECSSystem listener_system;
extern const MicroECSSystem lock_on_view_system;
extern const MicroECSSystem particles_system;
extern const MicroECSSystem physics_system;
extern const MicroECSSystem rendering_system;
extern const MicroECSSystem shaded_canvas_system;
extern const MicroECSSystem update_system;
extern const MicroECSSystem ui_system;
extern const MicroECSSystem tilemap_system;

int microSystemsUseAll();

#endif /* end of include guard: SYSTEMS_H */
