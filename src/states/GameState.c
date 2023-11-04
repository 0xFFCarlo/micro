#include "GameState.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../micro/System.h"

#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../entities/Drone.h"
#include "../entities/DroneLander.h"
#include "../entities/GUI.h"
#include "../entities/LogGUI.h"
#include "../entities/Planet.h"
#include "../entities/Player.h"
#include "../entities/Portal.h"
#include "../entities/Projectile.h"
#include "../entities/Space.h"
#include "../entities/Spawner.h"
#include "../misc/ambience_music.h"
#include "../misc/inventory.h"
#include "../systems/AnimationSystem.h"
#include "../systems/EventsSystem.h"
#include "../systems/FollowSystem.h"
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

void gameStateLoadResources()
{
  // Load atlas
  u32 atlasId = microResourceLoad("atlas", "res/textures/", "atlas");
  const u32 textureId = microTextureAtlasGetTextureId(atlasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  microResourceLoadFont("ui_font", "./res/fonts/firacode.ttf", 24,
                        MICRO_FILTER_NEAREST);

  // Load sounds
  microResourceLoad("explosion", "./res/sounds/explosion.wav", "sound");

  // Load sounds for robot
  microResourceLoad("robot_introduce", "./res/sounds/robot_say_introduce.wav",
                    "sound");
  microResourceLoad("robot_say_no", "./res/sounds/robot_say_no.wav", "sound");
  microResourceLoad("robot_say_yes", "./res/sounds/robot_say_yes.wav", "sound");
  microResourceLoad("robot_jump", "./res/sounds/robot-jump.wav", "sound");
  microResourceLoad("robot_footstep", "./res/sounds/footstep05.ogg", "sound");
  uint32_t gun_shot = microResourceLoad("gun_shot",
                                        "./res/sounds/laser-beam.mp3", "sound");
  microSoundSetVolume(gun_shot, 0.2);

  microResourceLoad("robot_recharging", "./res/sounds/recharging.wav", "sound");
  microResourceLoad("robot_alarm", "./res/sounds/alarm.wav", "sound");
  microResourceLoad("robot_shield_hit", "./res/sounds/shield-hit.wav", "sound");

  // Setup inventory system
  uint32_t pickup_sound = microResourceLoad("item_pickup_snd",
                                            "./res/sounds/robot_pickup.wav",
                                            "sound");
  inventorySetPickupSound(pickup_sound);
}

void gameStateInit()
{
  int windowWidth, windowHeight;
  microSystemGetWindowSize(&windowWidth, &windowHeight);

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
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(planetaryAligntmentSystem);
  microECSSystemAdd(followSystem);
  microECSSystemAdd(particlesSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(lifetimeSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);

  // Physics world
  int world_id = microPhysicsWorldNew();
  debug_print("Physics world_id: %d\n", world_id);

  // Load resources
  gameStateLoadResources();

  // Setup ambience
  ambienceMusicSetup();
  ambienceMusicSet(AMBIENCE_NORMAL);

  // Register entities
  SpaceEntityAdd();
  PlanetEntityAdd();
  PlayerEntityAdd();
  LogGUIAdd();
  GUIInit();
  SpawnerEntityAdd();

  int x, y;
  PlanetGetSurfacePosition(0.2, -24, &x, &y);
  // PortalAddEntity(x, y);

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
