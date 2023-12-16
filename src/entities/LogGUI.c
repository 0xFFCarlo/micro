#include "LogGUI.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../micro/State.h"
#include "../misc/layers.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const unsigned int top_pos_offset = 128 * 4;
const unsigned int left_pos_offset = 16;
const float log_gui_update_interval = 0.15;

int log_gui_entity_id = -1;
int font_id = -1;
char logText[512] = "FPS: 0";

float fps = 0.0;
int frames_count = 0;
float frames_time = 0.0;

float log_time = 0.0;
int log_entities_count = 0;
int log_bodies = 0;
int log_particles_count = 0;
RenderingDebugInfo debugInfo;

void handle_event(int entity, const SDL_Event *event)
{
  (void)(entity); // Unused parameter

  if (event->type == SDL_QUIT)
  {
    microStateQuit();
  }

  if (event->type == SDL_KEYDOWN)
  {
    if (event->key.keysym.scancode == SDL_SCANCODE_Q)
      microStateQuit();

    if (event->key.keysym.scancode == SDL_SCANCODE_L)
    {
      CDrawable *drawable = (CDrawable *)
        microECSEntityGetComponent(log_gui_entity_id, cid_drawable);
      drawable->visible = !drawable->visible;
    }
  }
}

void updateLogGUI(int entityId, float dt)
{
  (void)(entityId); // Unused parameter

  frames_count++;
  frames_time += dt;
  if (frames_time >= 1.0)
  {
    fps = frames_count / frames_time;
    frames_count = 0;
    frames_time = 0.0;
  }

  log_time += dt;
  if (log_time >= log_gui_update_interval)
  {
    log_time = 0.0;

    logText[0] = '\0';
    log_entities_count = microECSEntitiesCount();
    log_bodies = microPhysicsBodiesCount();
    log_particles_count = microParticlesCount();
    debugInfo = microGetRenderingDebugInfo();
    sprintf(logText,
            "FPS: %d\nEntities: %d\nBodies: %d\nDrawcalls: %d\nTriangles: "
            "%d\nTexture switch: %d\nShader switch: %d\nParticles: %d",
            (int)fps, log_entities_count, log_bodies, debugInfo.drawCalls,
            debugInfo.triangles, debugInfo.textureSwitches,
            debugInfo.shaderSwitches, log_particles_count);
  }
  microRenderingDebugInfoClear();
}

void LogGUIAdd()
{
  font_id = microResourceGet("ui_font");

  log_gui_entity_id = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(log_gui_entity_id, cid_position,
                             &(CPosition){.x = left_pos_offset,
                                          .y = top_pos_offset});
  microECSEntityAddComponent(log_gui_entity_id, cid_text,
                             &(CText){
                               .text = logText,
                               .lineSpacing = 3,
                               .alignment = TEXT_ALIGN_LEFT,
                               .fontId = font_id,
                             });
  microECSEntityAddComponent(log_gui_entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = LAYER_UI_2,
                               .visible = 0,
                             });
  microECSEntityAddComponent(log_gui_entity_id, cid_hud, NULL);
  microECSEntityAddComponent(log_gui_entity_id, cid_event_listener,
                             &(CEventListener){.on_event = handle_event});
  microECSEntityAddComponent(log_gui_entity_id, cid_update,
                             &(CUpdate){.update = updateLogGUI});
}
