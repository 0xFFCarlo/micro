#include "LogGUI.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

int log_gui_entity_id = -1;
int font_id = -1;
char logText[512] = "FPS: 0";

float fps = 0.0;
int frames_count = 0;
float frames_time = 0.0;

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
  frames_count++;
  frames_time += dt;
  if (frames_time >= 1.0) {
    fps = frames_count / frames_time;
    frames_count = 0;
    frames_time = 0.0;
  }
  logText[0] = '\0';
  int entities_count = microECSEntitiesCount();
  int bodies = microPhysicsBodiesCount();
  RenderingDebugInfo debugInfo = microGetRenderingDebugInfo();
  sprintf(logText, "FPS: %d\nEntities: %d\nBodies: %d\nDrawcalls: %d\nTriangles: %d\nTexture switch: %d\nShader switch: %d",
      (int)fps, entities_count, bodies, debugInfo.drawCalls, debugInfo.triangles, debugInfo.textureSwitches, debugInfo.shaderSwitches);
  microRenderingDebugInfoClear();
}


void LogGUIAdd()
{
  font_id = microFontLoadFromFile("./res/firacode.ttf", 14, MICRO_FILTER_NEAREST);

  int entityText = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entityText, cid_position, &(CPosition){
    .x = 16,
    .y = 16
  });
  microECSEntityAddComponent(entityText, cid_text, &(CText){
    .text = logText,
    .lineSpacing = 3,
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
