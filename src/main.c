#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

#include "Audio.h"
#include "Graphics.h"
#include "ECS.h"

#include "components/MotionComponents.h"
#include "components/RenderingComponents.h"
#include "components/LogicComponents.h"
#include "systems/RenderingSystem.h"
#include "systems/ShadedCanvasSystem.h"
#include "systems/UpdateSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/LockOnViewSystem.h"
#include "systems/EventsSystem.h"
#include "systems/PhysicsSystem.h"
#include "entities/Space.h"
#include "entities/Planet.h"
#include "entities/Player.h"

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

int main(int argc, char const *argv[])
{
  microGraphicsInit();

  int windowWidth, windowHeight;
  microWindowGetSize(&windowWidth, &windowHeight);

  microViewSet((MicroView){
      .viewportX = 0,
      .viewportY = 0,
      .viewportWidth = windowWidth,
      .viewportHeight = windowHeight,
      .centerX = windowWidth/2.0,
      .centerY = windowHeight/2.0,
      .width = windowWidth,
      .height = windowHeight,
      .rotation = 0,
      .flipY = 0
      });
  microViewApply();

  microECSInit();

  // Register components
  RegisterLogicComponents();
  RegisterMotionComponents();
  RegisterRenderingComponents();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(eventsSystem);
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);

  // Register entities
  PlayerEntityAdd();
  SpaceEntityAdd();
  PlanetEntityAdd();

  int font = microFontLoadFromFile("./res/FiraCode-Medium.ttf", 20, MICRO_FILTER_NEAREST);

  int entityText = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entityText, cid_position, &(CPosition){
    .x = 16,
    .y = 16
  });
  microECSEntityAddComponent(entityText, cid_text, &(CText){
    .text = "FPS: 0",
    .fontId = font,
  });
  microECSEntityAddComponent(entityText, cid_drawable, &(CDrawable){
    .layerId = 5
  });
  microECSEntityAddComponent(entityText, cid_hud, NULL);
  microECSEntityAddComponent(entityText, cid_event_listener, &(CEventListener){
    .on_event = handle_event
  });

  float deltaTime = 0.016;
  Uint32 lastTime = SDL_GetTicks();
  while (1)
  {
    lastTime = SDL_GetTicks();
    
    microGraphicsClear();
    
    microECSRun(deltaTime);
    
    microGraphicsDisplay();
    microSwapBuffers();

    //If frame finished early
    int frame_time = SDL_GetTicks() - lastTime;
    if (1000 / 70 > frame_time) {
      //Wait the remaining time
      SDL_Delay(1000 / 70 - frame_time);
    }

    // Calculate delta time
    frame_time = SDL_GetTicks() - lastTime;
    deltaTime = (float)frame_time / 1000.0;
  }
  
  microECSFree();
  microGraphicsQuit();

  return 0;
}
