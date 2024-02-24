#include "TestState.h"
#include "../micro/core/Audio.h"
#include "../micro/core/ECS.h"
#include "../micro/core/Graphics.h"
#include "../micro/core/Physics.h"
#include "../micro/core/Resources.h"
#include "../micro/core/System.h"

#include "../components/CustomComponents.h"
#include "../micro/components/LogicComponents.h"
#include "../micro/components/MotionComponents.h"
#include "../micro/components/RenderingComponents.h"
#include "../entities/Drone.h"
#include "../entities/DroneLander.h"
#include "../entities/GUI.h"
#include "../entities/LogGUI.h"
#include "../entities/Planet.h"
#include "../entities/Player.h"
#include "../entities/Projectile.h"
#include "../entities/Space.h"
#include "../entities/Spawner.h"
#include "../entities/Meteor.h"
#include "../misc/ambience_music.h"
#include "../misc/inventory.h"
#include "../micro/systems/AnimationSystem.h"
#include "../micro/systems/EventsSystem.h"
#include "../micro/systems/FollowSystem.h"
#include "../systems/GravitySystem.h"
#include "../systems/InteractionSystem.h"
#include "../micro/systems/LifetimeSystem.h"
#include "../micro/systems/LockOnViewSystem.h"
#include "../micro/systems/ParticlesSystem.h"
#include "../micro/systems/PhysicsSystem.h"
#include "../systems/PlanetaryAlignmentSystem.h"
#include "../micro/systems/RenderingSystem.h"
#include "../micro/systems/ShadedCanvasSystem.h"
#include "../micro/systems/UpdateSystem.h"
#include "../micro/util/debug.h"

void testStateLoadResources()
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
  microResourceLoad("robot_engine", "./res/sounds/robot-engine.mp3", "sound");

  // Setup inventory system
  uint32_t pickup_sound = microResourceLoad("item_pickup_snd",
      "./res/sounds/robot_pickup.wav",
      "sound");
  inventorySetPickupSound(pickup_sound);
}

void testStateInit()
{
  int windowWidth, windowHeight;
  microSystemGetWindowSize(&windowWidth, &windowHeight);

  int vpheight = 600;
  int vpwidth = vpheight * ((float)windowWidth / (float)windowHeight);
  microViewSet((MicroView){.viewportX = 0,
      .viewportY = 0,
      .viewportWidth = windowWidth,
      .viewportHeight = windowHeight,
      .centerX = 0,
      .centerY = 0,
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
  microECSSystemAdd(physicsSystem);
  microECSSystemAdd(followSystem);
  microECSSystemAdd(particlesSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(lifetimeSystem);
  microECSSystemAdd(animationSystem);
  microECSSystemAdd(renderingSystem);

  // Physics world
  int world_id = microPhysicsWorldNew();
  debug_print("Physics world_id: %d\n", world_id);

  // Load resources
  testStateLoadResources();

  // Setup ambience
  ambienceMusicSetup();
  ambienceMusicSet(AMBIENCE_NORMAL);

  // Register entities
  LogGUIAdd();

  // Create test object
  int anim_player_idle = microAnimationGet("robot-2-idle");
  int player_entity_id = microECSEntityNew(NULL, NULL);
  assert(player_entity_id != -1);

  CmpAddPosition(player_entity_id, 0, 0);

  int player_body_id = microPhysicsBodyNewCircle(player_entity_id, 0, 0, 0,
      32, 1.0, FALSE,
      FALSE, 0.0, 0.0);
  microPhysicsBodySetVelocity(player_body_id, -50.0, 50.0);
  CmpAddBody(player_entity_id, player_body_id);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

#define PLAYER_TEXTURE_WIDTH (48 * 3.0)
#define PLAYER_TEXTURE_HEIGHT (48 * 3.0)
  CmpAddSprite(player_entity_id, textureId, 0, 0, 16, 16);
  CmpAddTransform(player_entity_id, PLAYER_TEXTURE_WIDTH, PLAYER_TEXTURE_HEIGHT,
      PLAYER_TEXTURE_WIDTH / 2.0, PLAYER_TEXTURE_HEIGHT / 2.0, 45.0);
  CmpAddColor(player_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(player_entity_id, 10, 1);
  CmpAddAnimation(player_entity_id, anim_player_idle, 1.0, FALSE, FALSE, FALSE);
  CmpAddLockOnView(player_entity_id, 1);

  debug_print("Dinamically allocated memory: %llu bytes\n",
      memory_get_allocated());
}

void testStateUpdate(float dt)
{
  microGraphicsClear();
  microECSRun(dt);
  microGraphicsDisplay();
  microSwapBuffers();
}

void testStateFree()
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

MicroState testStateGet()
{
  MicroState state;
  state.init = testStateInit;
  state.update = testStateUpdate;
  state.free = testStateFree;
  return state;
}
