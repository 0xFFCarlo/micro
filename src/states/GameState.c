#include "GameState.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"

#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../entities/GUI.h"
#include "../entities/LogGUI.h"
#include "../entities/Planet.h"
#include "../entities/Player.h"
#include "../entities/Projectile.h"
#include "../entities/Space.h"
#include "../managers/ambience_music.h"
#include "../managers/inventory.h"
#include "../systems/AnimationSystem.h"
#include "../systems/EventsSystem.h"
#include "../systems/GravitySystem.h"
#include "../systems/InteractionSystem.h"
#include "../systems/LifetimeSystem.h"
#include "../systems/LockOnViewSystem.h"
#include "../systems/ParticlesSystem.h"
#include "../systems/PhysicsSystem.h"
#include "../systems/PlanetaryAlignmentSystem.h"
#include "../systems/RenderingSystem.h"
#include "../systems/ShadedCanvasSystem.h"
#include "../systems/UpdateSystem.h"
#include "../util/debug.h"

void gameStateInit()
{
  int windowWidth, windowHeight;
  microWindowGetSize(&windowWidth, &windowHeight);

  int vpheight = 600;
  int vpwidth = vpheight * ((float)windowWidth / (float)windowHeight);
  microViewSet((MicroView){.viewportX = 0,
                           .viewportY = 0,
                           .viewportWidth = windowWidth,
                           .viewportHeight = windowHeight,
                           .centerX = windowWidth / 2.0,
                           .centerY = windowHeight / 2.0,
                           .width = vpwidth,
                           .height = vpheight,
                           .rotation = 0,
                           .flipY = 0});
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
  microECSSystemAdd(interactionSystem);
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(lifetimeSystem);
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(planetaryAligntmentSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(particlesSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);

  // Physics world
  int world_id = microPhysicsWorldNew();
  debug_print("Physics world_id: %d\n", world_id);

  // Load resources
  microResourceLoad("atlas", "res/textures/", "atlas");
  microResourceLoadFont("ui_font", "./res/fonts/firacode.ttf", 24,
                        MICRO_FILTER_NEAREST);

  // Setup ambience
  ambienceMusicSetup();
  ambienceMusicSet(AMBIENCE_NORMAL);

  // Setup inventory system
  uint32_t pickup_sound = microResourceLoad("item_pickup_snd",
                                            "./res/sounds/robot_pickup.wav",
                                            "sound");
  inventorySetPickupSound(pickup_sound);

  // Register entities
  SpaceEntityAdd();
  PlanetEntityAdd();
  PlayerEntityAdd();
  LogGUIAdd();
  GUIInit();

  // Add projectile
  // float playerX, playerY;
  // PlayerGetPos(&playerX, &playerY);
  // ProjectileAddEntity(playerX - 32, playerY, 0, 0);

  debug_print("Dinamically allocated memory: %llu bytes\n",
              memory_get_allocated());
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
  microResourceFreeAll();
  microECSFree();
  debug_print("Freeing physics world\n");
  microGraphicsQuit();
  debug_print("Freeing graphics\n");
  microPhysicsWorldFreeAll();
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
