#include "LogGUI.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../Resources.h"
#include "../Graphics.h"
#include "../ECS.h"
#include "../Physics.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

int log_gui_entity_id = -1;
int font_id = -1;
char logText[512] = "FPS: 0";

void handle_event(int entity, SDL_Event event)
{
    if (event.type == SDL_QUIT) {
      exit(0);
    }

    if (event.type == SDL_KEYDOWN)
    {
      if (event.key.keysym.scancode == SDL_SCANCODE_Q)
        exit(0);
    }
}

void updateLogGUI(int entityId, float dt)
{
  logText[0] = '\0';
  int entities_count = microECSEntitiesCount();
  int worlds = microPhysicsWorldsCount();
  int bodies = microPhysicsBodiesCount();
  sprintf(logText, "FPS: %d\nEntities: %d\nWorlds: %d\nBodies: %d", (int)(1.0/dt), entities_count, worlds, bodies);
}


void LogGUIAdd()
{
  font_id = microFontLoadFromFile("./res/FiraCode-Medium.ttf", 16, MICRO_FILTER_NEAREST);

  int entityText = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entityText, cid_position, &(CPosition){
    .x = 16,
    .y = 16
  });
  microECSEntityAddComponent(entityText, cid_text, &(CText){
    .text = logText,
    .fontId = font_id,
  });
  microECSEntityAddComponent(entityText, cid_drawable, &(CDrawable){
    .layerId = 5
  });
  microECSEntityAddComponent(entityText, cid_hud, NULL);
  microECSEntityAddComponent(entityText, cid_event_listener, &(CEventListener){
    .on_event = handle_event
  });
  microECSEntityAddComponent(entityText, cid_update, &(CUpdate){
    .update = updateLogGUI
  });
}
