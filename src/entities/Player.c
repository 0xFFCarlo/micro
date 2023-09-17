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
#include "../micro/System.h"
#include "../systems/InteractionSystem.h"
#include "Planet.h"
#include "Projectile.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAYER_STATE_IDLE 0
#define PLAYER_STATE_WALK 1
#define PLAYER_STATE_SOLAR_CHARGING 2

#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1

#define PLAYER_TEXTURE_WIDTH (32 * 3)
#define PLAYER_TEXTURE_HEIGHT (32 * 3)
#define PLAYER_BODY_WIDTH (48 / 3.0)
#define PLAYER_BODY_HEIGHT (48)

int player_entity_id = -1;
int player_idle = -1;
int player_state = 0;
int player_direction = 0;

int anim_player_idle = -1;
int anim_player_walk = -1;
int anim_player_jump = -1;
int anim_player_idle_noenergy = -1;
int anim_player_walk_noenergy = -1;
int anim_player_jump_noenergy = -1;
int anim_player_solar_charging = -1;
int anim_shield = -1;

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
uint32_t robot_recharging = 0;
uint32_t robot_alarm = 0;
uint32_t robot_shield_hit = 0;

void PlayerCollide(int entityId, int otherEntityId)
{
  (void)entityId;      // unused parameter
  (void)otherEntityId; // unused parameter
}

void PlayerWalk()
{
  CAnimation *animation = CmpGetAnimation(player_entity_id);
  // Play footsteps sound
  if (microSoundIsPlaying(robot_footstep) == 0)
    microSoundPlay(robot_footstep, 1);
  microSoundStop(robot_recharging);

  if (animation->animationId != anim_player_walk &&
      animation->animationId != anim_player_walk_noenergy)
    animation->frameId = 0;

  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth * 0.1)
    animation->animationId = anim_player_walk_noenergy;
  else
    animation->animationId = anim_player_walk;

  animation->flipX = (player_direction != PLAYER_DIRECTION_RIGHT);
  animation->framesDuration = 0.25;
}

void PlayerJump()
{
  CAnimation *animation = CmpGetAnimation(player_entity_id);

  // Stop footsteps sound
  microSoundStop(robot_footstep);
  microSoundStop(robot_recharging);

  // Stop at the last frame
  if (animation->frameId == 1)
    animation->timeSinceLastFrame = 0;

  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth * 0.1)
    animation->animationId = anim_player_jump_noenergy;
  else
    animation->animationId = anim_player_jump;
  animation->flipX = (player_direction != PLAYER_DIRECTION_RIGHT);
  animation->framesDuration = 0.1;
}

void PlayerChargeWithSolarPanel(float dt)
{
  CAnimation *animation = CmpGetAnimation(player_entity_id);

  // Stop footsteps sound
  microSoundStop(robot_footstep);

  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth)
  {
    health->health += 1.0 * dt;
    if (microSoundIsPlaying(robot_recharging) == 0)
      microSoundPlay(robot_recharging, 0);
  }
  else
  {
    health->health = health->maxHealth;
    if (microSoundIsPlaying(robot_recharging) == 1)
      microSoundStop(robot_recharging);
  }

  if (animation->animationId != anim_player_solar_charging &&
      animation->animationId != anim_player_idle)
    animation->frameId = 0;
  animation->animationId = anim_player_solar_charging;
  animation->framesDuration = 1.0;
}

void PlayerGetSize(float *width, float *height)
{
  *width = PLAYER_BODY_WIDTH;
  *height = PLAYER_BODY_HEIGHT;
}

void PlayerShootAt(float x, float y)
{
  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < 1.0)
  {
    microSoundPlay(robot_say_no, 0);
    return;
  }

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

  microSoundPlayNewChannel(gun_shot, 0);
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
  else if (event->type == SDL_KEYDOWN)
  {
    if (event->key.keysym.scancode == SDL_SCANCODE_P)
    {
      PlayerShieldHit(1.0);
    }
  }
}

void PlayerIdle()
{
  CAnimation *animation = CmpGetAnimation(player_entity_id);

  // Stop footsteps sound
  microSoundStop(robot_footstep);

  if (animation->animationId != anim_player_idle &&
      animation->animationId != anim_player_idle_noenergy)
    animation->frameId = 0;

  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth * 0.1)
    animation->animationId = anim_player_idle_noenergy;
  else
    animation->animationId = anim_player_idle;

  animation->flipX = (player_direction != PLAYER_DIRECTION_RIGHT);
  animation->framesDuration = 1.0;
}

void playerUpdate(int entityId, float dt)
{
  (void)entityId; // unused parameter

  // Controls and player state
  player_state = PLAYER_STATE_IDLE;
  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
  {
    player_direction = PLAYER_DIRECTION_RIGHT;
    player_state = PLAYER_STATE_WALK;
  }
  else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
  {
    player_direction = PLAYER_DIRECTION_LEFT;
    player_state = PLAYER_STATE_WALK;
  }

  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
    playerJump = 1;

  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_C])
  {
    CHealth *health = CmpGetHealth(player_entity_id);
    if (health->health < health->maxHealth)
      player_state = PLAYER_STATE_SOLAR_CHARGING;
  }

  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E])
    interactionSystemInteract();

  float viewX, viewY;
  float viewWidth, viewHeight;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);

  CPosition *position = CmpGetPosition(player_entity_id);

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

  // Energy alarm
  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth * 0.1)
  {
    if (microSoundIsPlaying(robot_alarm) == 0)
      microSoundPlay(robot_alarm, 1);
  }
  else
  {
    microSoundStop(robot_alarm);
  }

  // Update player based on state
  if (playerJump == 1)
    PlayerJump();
  else if (player_state == PLAYER_STATE_WALK)
    PlayerWalk();
  else if (player_state == PLAYER_STATE_SOLAR_CHARGING)
    PlayerChargeWithSolarPanel(dt);
  else
    PlayerIdle();

  // Update Planet alignment component
  CPlanetaryAlignment *pa = CmpGetPlanetaryAlignment(player_entity_id);
  pa->planet_x = planetX;
  pa->planet_y = planetY;
}

void PlayerGetPos(float *x, float *y)
{
  CPosition *position = CmpGetPosition(player_entity_id);
  *x = position->x;
  *y = position->y;
}

float PlayerGetRotation()
{
  return playerRotation;
}

float PlayerGetHealth()
{
  return CmpGetHealth(player_entity_id)->health;
}

float PlayerGetMaxHealth()
{
  return CmpGetHealth(player_entity_id)->maxHealth;
}

void ShieldUpdate(int shieldId, float dt)
{
  (void)dt; // unused parameter
  CPosition *position = CmpGetPosition(shieldId);
  CPosition *playerPosition = CmpGetPosition(player_entity_id);
  position->x = playerPosition->x;
  position->y = playerPosition->y;
}

void PlayerShieldHit(float damage)
{
  CHealth *health = CmpGetHealth(player_entity_id);
  health->health -= damage;
  if (health->health < 0.0)
    health->health = 0.0;

  // Play sound
  microSoundPlay(robot_shield_hit, 0);

  int shield_id = microECSEntityNew(NULL, NULL);
  assert(shield_id != -1);
  CPosition *position = CmpGetPosition(player_entity_id);
  CmpAddPosition(shield_id, position->x, position->y);
  CmpAddUpdate(shield_id, ShieldUpdate);
  CmpAddDrawable(shield_id, 5, 1);
  CmpAddTransform(shield_id, PLAYER_TEXTURE_WIDTH, PLAYER_TEXTURE_HEIGHT,
                  PLAYER_TEXTURE_WIDTH / 2.0, PLAYER_TEXTURE_HEIGHT / 2.0, 0.0);
  CmpAddSprite(shield_id, microResourceGet("atlas"), 0, 0, 16, 16);
  CmpAddAnimation(shield_id, anim_shield, 0.1, 0, 0);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddLifetime(shield_id, 0.4);
  CmpAddFollow(shield_id, player_entity_id, 1, 0, 0);
}

void PlayerEntityAdd()
{
  anim_player_idle = microAnimationGet("robot-idle");
  anim_player_walk = microAnimationGet("robot-walk");
  anim_player_jump = microAnimationGet("robot-jump");
  anim_player_idle_noenergy = microAnimationGet("robot-idle-noenergy");
  anim_player_walk_noenergy = microAnimationGet("robot-walk-noenergy");
  anim_player_jump_noenergy = microAnimationGet("robot-jump-noenergy");
  anim_player_solar_charging = microAnimationGet("robot-solarpanel");
  anim_shield = microAnimationGet("shield");

  player_entity_id = microECSEntityNew(NULL, NULL);
  assert(player_entity_id != -1);

  // Position component
  int viewportWidth, viewportHeight;
  microSystemGetWindowSize(&viewportWidth, &viewportHeight);
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
  CmpAddUpdate(player_entity_id, playerUpdate);
  CmpAddLockOnView(player_entity_id, 1);

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(player_entity_id, planetX, planetY);

  CmpAddHealth(player_entity_id, 16, 16);
  CmpAddEventListener(player_entity_id, playerEventListener);

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

  gun_shot = microResourceLoad("gun_shot", "./res/sounds/laser-beam.mp3",
                               "sound");

  robot_recharging = microResourceLoad("robot_recharging",
                                       "./res/sounds/recharging.wav", "sound");

  robot_alarm = microResourceLoad("robot_alarm", "./res/sounds/alarm.wav",
                                  "sound");

  robot_shield_hit = microResourceLoad("robot_shield_hit",
                                       "./res/sounds/shield-hit.wav", "sound");

  microSoundPlay(robot_say_introduce, 0);
}
