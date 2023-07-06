#include "GameState.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"

#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../components/CustomComponents.h"
#include "../systems/RenderingSystem.h"
#include "../systems/ShadedCanvasSystem.h"
#include "../systems/UpdateSystem.h"
#include "../systems/AnimationSystem.h"
#include "../systems/LockOnViewSystem.h"
#include "../systems/EventsSystem.h"
#include "../systems/PhysicsSystem.h"
#include "../systems/PlanetaryAlignmentSystem.h"
#include "../systems/GravitySystem.h"
#include "../systems/ParticlesSystem.h"
#include "../entities/Space.h"
#include "../entities/Planet.h"
#include "../entities/Player.h"
#include "../entities/LogGUI.h"
#include "../util/mem_debug.h"

void gameStateInit()
{
  int windowWidth, windowHeight;
  microWindowGetSize(&windowWidth, &windowHeight);
  
  int vpheight = 600;
  int vpwidth = vpheight * ((float)windowWidth/ (float)windowHeight);
  microViewSet((MicroView){
      .viewportX = 0,
      .viewportY = 0,
      .viewportWidth = windowWidth,
      .viewportHeight = windowHeight,
      .centerX = windowWidth/2.0,
      .centerY = windowHeight/2.0,
      .width = vpwidth,
      .height = vpheight,
      .rotation = 0,
      .flipY = 0
      });
  microViewApply();

  // Register components
  RegisterLogicComponents();
  RegisterMotionComponents();
  RegisterRenderingComponents();
  RegisterCustomComponents();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(eventsSystem);
  microECSSystemAdd(gravitySystem);
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(planetaryAligntmentSystem);
  microECSSystemAdd(particlesSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);

  // Physics world
  int world_id = microPhysicsWorldNew();
  printf("Physics world_id: %d\n", world_id);

  // Texture atlas
  microResourceLoad("atlas", "res/textures/", "atlas");
  
  // Register entities
  SpaceEntityAdd();
  PlanetEntityAdd();
  PlayerEntityAdd();
  LogGUIAdd();
}

void gameStateUpdate(float dt)
{
  microGraphicsClear();
  microECSRun(dt);
  microGraphicsDisplay();
  microSwapBuffers();
}

void gameStateFree()
{
  microResourceFree("atlas");
  microECSFree();
  microAnimationFreeAll();
  microFontFreeAll();

  memory_check_leaks();
  exit(0);
}

MicroState gameStateGet()
{
  MicroState state;
  state.init = gameStateInit;
  state.update = gameStateUpdate;
  state.free = gameStateFree;
  return state;
}

