#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

#include "Audio.h"
#include "Graphics.h"
#include "ECS.h"
#include "Physics.h"

#include "components/MotionComponents.h"
#include "components/RenderingComponents.h"
#include "components/LogicComponents.h"
#include "components/CustomComponents.h"
#include "systems/RenderingSystem.h"
#include "systems/ShadedCanvasSystem.h"
#include "systems/UpdateSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/LockOnViewSystem.h"
#include "systems/EventsSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/PlanetaryAlignmentSystem.h"
#include "entities/Space.h"
#include "entities/Planet.h"
#include "entities/Player.h"
#include "entities/LogGUI.h"

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
  RegisterCustomComponents();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(eventsSystem);
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(planetaryAligntmentSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);


  // Physics world
  int world_id = microPhysicsWorldNew();
  printf("world_id: %d\n", world_id);

  // Register entities
  PlayerEntityAdd();
  SpaceEntityAdd();
  PlanetEntityAdd();
  LogGUIAdd();

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
