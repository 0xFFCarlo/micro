#include "../core/ECS.h"
#include "../core/UI.h"
#include <stdlib.h>

static void ui_system_update(float dt)
{
  (void)dt;
  microUIUpdate();
}

MicroECSSystem ui_system = {ui_system_update, NULL, NULL};
