#include "../core/Tilemap.h"
#include "../core/ECS.h"
#include <stdlib.h>

static void tilemap_system_update(float dt)
{
  microTilemapsUpdate(dt);
}

MicroECSSystem tilemap_system = {tilemap_system_update, NULL, NULL, NULL};
