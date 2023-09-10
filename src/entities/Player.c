#include "Player.h"
#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../systems/InteractionSystem.h"
#include "Planet.h"
#include "Projectile.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAYER_STATE_IDLE 0
#define PLAYER_STATE_WALK 1

#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1

#define PLAYER_TEXTURE_WIDTH 48
#define PLAYER_TEXTURE_HEIGHT 48
#define PLAYER_BODY_WIDTH (PLAYER_TEXTURE_WIDTH / 3.0)
#define PLAYER_BODY_HEIGHT (PLAYER_TEXTURE_HEIGHT)

int player_entity_id = -1;
int player_idle = -1;
int player_state = 0;
int player_direction = 0;

int anim_player_idle = -1;
int anim_player_walk = -1;
int anim_player_jump = -1;

float toPlanetNormX = 0.0;
float toPlanetNormY = 0.0;

float playerRotation = 0.0;
int playerJump = 0;

int player_body_id = -1;

// Sounds ids
uint32_t robot_footstep = 0;
uint32_t robot_say_introduce = 0;
uint32_t robot_say_yes = 0;
uint32_t robot_say_no = 0;
uint32_t robot_jump = 0;
uint32_t gun_shot = 0;

void PlayerCollide(int entityId, int otherEntityId)
{
  (void)entityId;      // unused parameter
  (void)otherEntityId; // unused parameter
}

void PlayerMove(int direction)
{
  if (direction == PLAYER_DIRECTION_RIGHT)
  {
    player_direction = PLAYER_DIRECTION_RIGHT;
    player_state = PLAYER_STATE_WALK;
  }
  else
  {
    player_direction = PLAYER_DIRECTION_LEFT;
    player_state = PLAYER_STATE_WALK;
  }
}

void PlayerJump()
{
  playerJump = 1;
}

void PlayerGetSize(float *width, float *height)
{
  *width = PLAYER_BODY_WIDTH;
  *height = PLAYER_BODY_HEIGHT;
}

void PlayerShootAt(float x, float y)
{
  CHealth *health = (CHealth *)microECSEntityGetComponent(player_entity_id,
                                                          cid_health);
  if (health->health < 1.0)
    return;

  health->health -= 1;

  float playerX, playerY;
  PlayerGetPos(&playerX, &playerY);

  float toMouseX = x - playerX;
  float toMouseY = y - playerY;
  float toMouseDist = sqrt(toMouseX * toMouseX + toMouseY * toMouseY);
  toMouseX /= toMouseDist;
  toMouseY /= toMouseDist;
  float forceX = toMouseX * 600.0;
  float forceY = toMouseY * 600.0;

  ProjectileAddEntity(playerX, playerY, forceX, forceY);

  microSoundPlay(gun_shot, 0);
}

void playerEventListener(int entity, const SDL_Event *event)
{
  (void)entity; // unused parameter

  // Mouse press
  if (event->type == SDL_MOUSEBUTTONDOWN)
  {
    if (event->button.button == SDL_BUTTON_LEFT)
    {
      int mouse_x = 0, mouse_y = 0;
      SDL_GetMouseState(&mouse_x, &mouse_y);
      float px, py;
      microViewPointScreenToWorld(mouse_x, mouse_y, &px, &py);
      PlayerShootAt(px, py);
    }
  }
}

void playerUpdate(int entityId, float dt)
{
  (void)entityId; // unused parameter

  // Controls
  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
    PlayerMove(0);
  else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
    PlayerMove(1);
  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
    PlayerJump();

  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E])
    interactionSystemInteract();

  float viewX, viewY;
  float viewWidth, viewHeight;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);

  CPosition *position = (CPosition *)
    microECSEntityGetComponent(player_entity_id, cid_position);

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  float toPlanetX = planetX - position->x;
  float toPlanetY = planetY - position->y;
  float toPlanetDist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
  toPlanetNormX = toPlanetX / toPlanetDist;
  toPlanetNormY = toPlanetY / toPlanetDist;
  float playerHeight = PlanetGetRadius() - GROUND_HEIGHT * 2 +
                       PLAYER_BODY_HEIGHT / 2.0;

  float forceX = toPlanetNormX * 250.0;
  float forceY = toPlanetNormY * 250.0;
  // float forceX, forceY;
  // microPhysicsBodyGetForce(player_body_id, &forceX, &forceY);

  // Motion on planet
  if (toPlanetDist <= playerHeight + 1.0)
  {

    // float px, py;
    // microPhysicsBodyGetPosition(player_body_id, &px, &py);
    float vx, vy;
    microPhysicsBodyGetVelocity(player_body_id, &vx, &vy);

    // Motion
    if (player_state == PLAYER_STATE_WALK)
    {
      if (player_direction == PLAYER_DIRECTION_RIGHT)
      {
        forceX += toPlanetNormY * 150000.0 * dt;
        forceY += -toPlanetNormX * 150000.0 * dt;
      }
      else
      {
        forceX += -toPlanetNormY * 150000.0 * dt;
        forceY += toPlanetNormX * 150000.0 * dt;
      }
    }

    if (playerJump)
    {
      forceX += -toPlanetNormX * 4000.0;
      forceY += -toPlanetNormY * 4000.0;
      playerJump = 0;
    }

    vx *= 0.70;
    vy *= 0.70;
    microPhysicsBodySetVelocity(player_body_id, vx, vy);

    // microPhysicsBodySetPosition(player_body_id, px, py);
  }

  microPhysicsBodySetForce(player_body_id, forceX, forceY);

  // Update animation
  CAnimation *animation = (CAnimation *)
    microECSEntityGetComponent(player_entity_id, cid_animation);
  if (playerJump == 1)
  {
    // Stop at the last frame
    if (animation->frameId == 1)
      animation->timeSinceLastFrame = 0;

    animation->framesDuration = 0.1;

    animation->animationId = anim_player_jump;
    if (player_direction == PLAYER_DIRECTION_RIGHT)
      animation->flipX = 0;
    else
      animation->flipX = 1;

    // Stop footsteps sound
    microSoundStop(robot_footstep);
  }
  else if (player_state == PLAYER_STATE_WALK)
  {
    // Play footsteps sound
    if (microSoundIsPlaying(robot_footstep) == 0)
      microSoundPlay(robot_footstep, 1);

    if (player_direction == PLAYER_DIRECTION_RIGHT)
    {
      if (animation->animationId != anim_player_walk)
        animation->frameId = 0;
      animation->animationId = anim_player_walk;
      animation->flipX = 0;
    }
    else
    {
      if (animation->animationId != anim_player_walk)
        animation->frameId = 0;
      animation->animationId = anim_player_walk;
      animation->flipX = 1;
    }
    animation->framesDuration = 0.25;
  }
  else
  {
    // Stop footsteps sound
    microSoundStop(robot_footstep);

    if (player_direction == PLAYER_DIRECTION_RIGHT)
    {
      if (animation->animationId != anim_player_idle)
        animation->frameId = 0;
      animation->animationId = anim_player_idle;
      animation->flipX = 0;
    }
    else
    {
      if (animation->animationId != anim_player_idle)
        animation->frameId = 0;
      animation->animationId = anim_player_idle;
      animation->flipX = 1;
    }
    animation->framesDuration = 1.0;
  }
  player_state = PLAYER_STATE_IDLE;

  CPlanetaryAlignment *pa = (CPlanetaryAlignment *)
    microECSEntityGetComponent(player_entity_id, cid_planetary_alignment);
  pa->planet_x = planetX;
  pa->planet_y = planetY;
}

void PlayerGetPos(float *x, float *y)
{
  CPosition *position = (CPosition *)
    microECSEntityGetComponent(player_entity_id, cid_position);
  *x = position->x;
  *y = position->y;
}

float PlayerGetRotation()
{
  return playerRotation;
}

int PlayerGetHealth()
{
  CHealth *health = (CHealth *)microECSEntityGetComponent(player_entity_id,
                                                          cid_health);
  return health->health;
}

int PlayerGetMaxHealth()
{
  CHealth *health = (CHealth *)microECSEntityGetComponent(player_entity_id,
                                                          cid_health);
  return health->maxHealth;
}

void PlayerEntityAdd()
{
  anim_player_idle = microAnimationGet("robot-idle");
  anim_player_walk = microAnimationGet("robot-walk");
  anim_player_jump = microAnimationGet("robot-jump");

  player_entity_id = microECSEntityNew(NULL, NULL);
  assert(player_entity_id != -1);

  // Position component
  int viewportWidth, viewportHeight;
  microWindowGetSize(&viewportWidth, &viewportHeight);
  int x, y;
  x = viewportWidth / 2.0;
  y = viewportHeight / 2.0 - 100.0;
  CmpAddPosition(player_entity_id, x, y);

  // Body component
  // player_body_id = microPhysicsBodyNewCircle(0, viewportWidth / 2.0,
  //                                            viewportHeight / 2.0 - 100.0,
  //                                            PLAYER_WIDTH / 2.0, 1.0, 0, 0,
  //                                            0.0, 0.0);
  player_body_id = microPhysicsBodyNewRect(player_entity_id, 0, x, y,
                                           PLAYER_BODY_WIDTH,
                                           PLAYER_BODY_HEIGHT, 1.0, 0, 0, 0.0,
                                           0.0);
  microPhysicsBodySetCollisionCallback(player_body_id, PlayerCollide);
  microPhysicsBodySetFilter(player_body_id, 2, 1);
  CmpAddBody(player_entity_id, player_body_id);

  // Sprite component
  // int textureId = microTextureLoadFromFile("./res/robot.png");
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  CmpAddSprite(player_entity_id, textureId, 0, 0, 16, 16);
  CmpAddTransform(player_entity_id, PLAYER_TEXTURE_WIDTH, PLAYER_TEXTURE_HEIGHT,
                  PLAYER_TEXTURE_WIDTH / 2.0, PLAYER_TEXTURE_HEIGHT / 2.0, 0.0);
  CmpAddColor(player_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(player_entity_id, 4, 1);
  CmpAddAnimation(player_entity_id, anim_player_idle, 1.0, 0, 0);

  // Update component
  microECSEntityAddComponent(player_entity_id, cid_update,
                             &(CUpdate){
                               .update = playerUpdate,
                             });

  CmpAddLockOnView(player_entity_id, 1);

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(player_entity_id, planetX, planetY);

  CmpAddHealth(player_entity_id, 16, 16);

  // Event
  microECSEntityAddComponent(player_entity_id, cid_event_listener,
                             &(CEventListener){
                               .on_event = playerEventListener,
                             });

  interactionSystemSetActorId(player_entity_id);

  // Load sounds for robot
  robot_say_introduce = microResourceLoad(
    "robot_introduce", "./res/sounds/robot_say_introduce.wav", "sound");
  robot_say_no = microResourceLoad("robot_say_no",
                                   "./res/sounds/robot_say_no.wav", "sound");
  robot_say_yes = microResourceLoad("robot_say_yes",
                                    "./res/sounds/robot_say_yes.wav", "sound");
  robot_jump = microResourceLoad("robot_jump", "./res/sounds/jump.wav",
                                 "sound");

  robot_footstep = microResourceLoad("robot_footstep",
                                     "./res/sounds/footstep05.ogg", "sound");

  gun_shot = microResourceLoad("gun_shot", "./res/sounds/shooting_1.mp3",
                               "sound");

  microSoundPlay(robot_say_introduce, 0);
}
