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
#include "../misc/collision.h"
#include "../misc/layers.h"
#include "Explosion.h"
#include "../systems/InteractionSystem.h"
#include "GUI.h"
#include "Spawner.h"
#include "Planet.h"
#include "Space.h"
#include "Projectile.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1

#define PLAYER_TEXTURE_WIDTH (48 * 2.5)
#define PLAYER_TEXTURE_HEIGHT (48 * 2.5)
#define PLAYER_BODY_RADIUS (48 / 2.25)

#define PLAYER_JUNMP_FORCE 5000.0
#define PLAYER_MOVE_FORCE 150000.0

int player_entity_id = -1;
bool player_alive = 1;
int player_idle = -1;
int player_state = 0;
int player_direction = 0;

int anim_player_idle = -1;
int anim_player_fly = -1;
int anim_player_walk = -1;
int anim_player_jump = -1;
int anim_player_idle_noenergy = -1;
int anim_player_walk_noenergy = -1;
int anim_player_jump_noenergy = -1;
int anim_player_solar_charging = -1;
int anim_shield = -1;

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
uint32_t robot_engine = 0;

void PlayerCollide(int playerId, int otherEntityId)
{
  (void)playerId; // unused parameter
  CBody *bodyOther = CmpGetBody(otherEntityId);
  f32 vx, vy;
  microPhysicsBodyGetVelocity(bodyOther->body_id, &vx, &vy);
  f32 speed = sqrt(vx * vx + vy * vy);
  if (speed > 300)
    PlayerHit(1.0);
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

  if (player_direction == PLAYER_DIRECTION_RIGHT)
    animation->reverse = FALSE;
  else
    animation->reverse = TRUE;

  animation->duration = 0.25;
}

void PlayerJump()
{
  CAnimation *animation = CmpGetAnimation(player_entity_id);

  // Stop footsteps sound
  microSoundStop(robot_footstep);
  microSoundStop(robot_recharging);

  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < health->maxHealth * 0.1)
    animation->animationId = anim_player_jump_noenergy;
  else
    animation->animationId = anim_player_jump;
  animation->duration = 0.1;
  animation->reverse = FALSE;
}

void PlayerFly() {
  CAnimation *animation = CmpGetAnimation(player_entity_id);
  
  microSoundStop(robot_footstep);
  microSoundStop(robot_recharging);

  if (microSoundIsPlaying(robot_engine) == FALSE)
    microSoundPlay(robot_engine, TRUE);

  animation->animationId = anim_player_fly;
  animation->duration = 0.1;
  animation->reverse = FALSE;
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
  animation->duration = 1.0;
  animation->reverse = FALSE;
}

void PlayerGetSize(float *width, float *height)
{
  *width = PLAYER_BODY_RADIUS;
  *height = PLAYER_BODY_RADIUS;
}

void PlayerShootAt(float x, float y)
{
  CHealth *health = CmpGetHealth(player_entity_id);
  if (health->health < 1.0)
  {
    microSoundPlay(robot_say_no, 0);
    return;
  }

  health->health -= 0.2;

  float playerX, playerY;
  PlayerGetPos(&playerX, &playerY);

  const CTransform *transform = CmpGetTransform(player_entity_id);
  const f32 eyeOffsetX = cos(transform->rotation - M_PI / 2) * 8.0;
  const f32 eyeOffsetY = sin(transform->rotation - M_PI / 2) * 8.0;

  f32 toMouseX = x - (playerX + eyeOffsetX);
  f32 toMouseY = y - (playerY + eyeOffsetY);
  const f32 toMouseDist = sqrt(toMouseX * toMouseX + toMouseY * toMouseY);
  toMouseX /= toMouseDist;
  toMouseY /= toMouseDist;
  float forceX = toMouseX * 600.0;
  float forceY = toMouseY * 600.0;

  ProjectileAddEntity(PROJECTILE_TYPE_PLAYER, playerX + eyeOffsetX,
                      playerY + eyeOffsetY, forceX, forceY);

  microSoundPlayNewChannel(gun_shot, 0);
}

void playerEventListener(int entity, const SDL_Event *event)
{
  (void)entity; // unused parameter
  
  // Cannot do anything while landing or departing
  if (player_state == PLAYER_STATE_LANDING ||
      player_state == PLAYER_STATE_DEPARTING ||
      player_state == PLAYER_STATE_WARP_DRIVE)
    return;

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
  
  animation->reverse = FALSE;
  animation->duration = 1.0;
}

void playerUpdate(int entityId, float dt)
{
  (void)entityId; // unused parameter
  
  CPosition *position = CmpGetPosition(player_entity_id);

  // Controls and player state
  if (player_state != PLAYER_STATE_LANDING &&
      player_state != PLAYER_STATE_DEPARTING &&
      player_state != PLAYER_STATE_WARP_DRIVE &&
      player_state != PLAYER_STATE_DEAD)
  {
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
    {
      if (playerJump == 0)
      {
        microSoundPlay(robot_jump, 0);
        playerJump = 1;
      }
    }

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

    float planetX, planetY;
    PlanetGetPos(&planetX, &planetY);
    float toPlanetX = planetX - position->x;
    float toPlanetY = planetY - position->y;
    float toPlanetDist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
    float toPlanetNormX = toPlanetX / toPlanetDist;
    float toPlanetNormY = toPlanetY / toPlanetDist;
    float playerHeight = PlanetGetRadius() - GROUND_HEIGHT * 2 +
      PLAYER_BODY_RADIUS;

    float forceX = toPlanetNormX * 250.0;
    float forceY = toPlanetNormY * 250.0;
    // float forceX, forceY;
    // microPhysicsBodyGetForce(player_body_id, &forceX, &forceY);

    // Motion on planet
    if (toPlanetDist <= playerHeight)
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
          forceX += toPlanetNormY * PLAYER_MOVE_FORCE * dt;
          forceY += -toPlanetNormX * PLAYER_MOVE_FORCE * dt;
        }
        else
        {
          forceX += -toPlanetNormY * PLAYER_MOVE_FORCE * dt;
          forceY += toPlanetNormX * PLAYER_MOVE_FORCE * dt;
        }
      }

      if (playerJump)
      {
        forceX += -toPlanetNormX * PLAYER_JUNMP_FORCE;
        forceY += -toPlanetNormY * PLAYER_JUNMP_FORCE;
        playerJump = 0;
      }

      vx *= 0.70;
      vy *= 0.70;
      microPhysicsBodySetVelocity(player_body_id, vx, vy);

      // microPhysicsBodySetPosition(player_body_id, px, py);
    }

    microPhysicsBodySetForce(player_body_id, forceX, forceY);

  }
  else if (player_state == PLAYER_STATE_LANDING)
  {
    float planetX, planetY;
    PlanetGetPos(&planetX, &planetY);
    float deltaX = planetX - position->x;
    float deltaY = planetY - position->y;
    float planetDist = sqrt(deltaX * deltaX + deltaY * deltaY);
    float toDirX = deltaX / planetDist;
    float toDirY = deltaY / planetDist;
    const float surfaceDist = planetDist - PlanetGetRadius();
    float speed = fmin(surfaceDist, 400);
    speed = fmax(speed, 100);

    float velX = toDirX * speed;
    float velY = toDirY * speed;

    microPhysicsBodySetVelocity(player_body_id, velX, velY);

    if (surfaceDist < 10.0) {
      player_state = PLAYER_STATE_IDLE;
      microSoundStop(robot_engine);
      microSoundPlay(robot_say_introduce, 0);
      GUIPlayerSay("I am robot.");
      SpawnerStart(); 
    }
  }
  else if (player_state == PLAYER_STATE_DEPARTING)
  {
    float planetX, planetY;
    PlanetGetPos(&planetX, &planetY);
    float deltaX = planetX - position->x;
    float deltaY = planetY - position->y;
    float planetDist = sqrt(deltaX * deltaX + deltaY * deltaY);
    float toDirX = -deltaX / planetDist;
    float toDirY = -deltaY / planetDist;
    const float surfaceDist = planetDist - PlanetGetRadius();
    float speed = fmin(surfaceDist, 400);
    speed = fmax(speed, 100);

    if (surfaceDist >= 500.0) {
      speed = 0.0;
      player_state = PLAYER_STATE_WARP_DRIVE;
      SpawnerStop();
      SpawnerClear();
      SpaceWarpDriveStart();
    }

    float velX = toDirX * speed;
    float velY = toDirY * speed;

    microPhysicsBodySetVelocity(player_body_id, velX, velY);
  }
  else if (player_state == PLAYER_STATE_WARP_DRIVE)
  {
    if (SpaceIsWarping() == 0) {
      PlanetGenerate();
      player_state = PLAYER_STATE_LANDING;
    }
  }
  else if (player_state == PLAYER_STATE_DEAD)
  {
    CDrawable *drawable = CmpGetDrawable(player_entity_id);
    drawable->visible = FALSE;
  }
    
  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_P])
    player_state = PLAYER_STATE_DEPARTING;

  if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_I])
    player_state = PLAYER_STATE_LANDING;


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
  if (player_state == PLAYER_STATE_LANDING || player_state == PLAYER_STATE_DEPARTING 
      || player_state == PLAYER_STATE_WARP_DRIVE)
    PlayerFly();
  else if (playerJump == 1)
    PlayerJump();
  else if (player_state == PLAYER_STATE_WALK)
    PlayerWalk();
  else if (player_state == PLAYER_STATE_SOLAR_CHARGING)
    PlayerChargeWithSolarPanel(dt);
  else
    PlayerIdle();

  // Update Planet alignment component
  CPlanetaryAlignment *pa = CmpGetPlanetaryAlignment(player_entity_id);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  pa->planet_x = planetX;
  pa->planet_y = planetY;
}

void PlayerGetPos(float *x, float *y)
{
  if (!player_alive) return;
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

void PlayerFree(int entityId)
{
  PlayerData* data = microECSEntityGetData(entityId); 
  free(data);
}

bool PlayerIsAlive()
{
  return player_alive;
}

void PlayerHit(float damage)
{
  CPosition *position = CmpGetPosition(player_entity_id);

  CHealth *health = CmpGetHealth(player_entity_id);
  health->health -= damage;
  if (health->health <= 0.0) {
    health->health = 0.0;
    if (player_state != PLAYER_STATE_DEAD) {
      makeExplosionDroneHit(position->x, position->y, 0, 0, 20, TRUE);
      uint32_t explosion_snd = microResourceGet("explosion");
      microSoundPlay(explosion_snd, 0);
      player_state = PLAYER_STATE_DEAD;
    }
    return;
  }

  // Play sound
  microSoundPlay(robot_shield_hit, 0);

  PlayerData *playerData = malloc(sizeof(PlayerData));
  int shield_id = microECSEntityNew(playerData, PlayerFree);
  assert(shield_id != -1);
  CmpAddPosition(shield_id, position->x, position->y);
  CmpAddUpdate(shield_id, ShieldUpdate);
  CmpAddDrawable(shield_id, 5, 1);
  CmpAddTransform(shield_id, PLAYER_TEXTURE_WIDTH/1.5, PLAYER_TEXTURE_HEIGHT/1.5,
                  PLAYER_TEXTURE_WIDTH / 3.0, PLAYER_TEXTURE_HEIGHT / 3.0, 0.0);
  CmpAddSprite(shield_id, microResourceGet("atlas"), 0, 0, 16, 16);
  CmpAddAnimation(shield_id, anim_shield, 0.1, FALSE, FALSE, FALSE);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddLifetime(shield_id, 0.4);
  CmpAddFollow(shield_id, player_entity_id, 1, 0, 0);
}

PlayerState PlayerGetState()
{
  return player_state;
}

void PlayerEntityAdd(const float x, const float y)
{
  anim_player_idle = microAnimationGet("robot-2-idle");
  anim_player_fly = microAnimationGet("robot-2-fly");
  anim_player_walk = microAnimationGet("robot-2-walk");
  anim_player_jump = microAnimationGet("robot-2-jump");
  anim_player_idle_noenergy = microAnimationGet("robot-2-idle-noenergy");
  anim_player_walk_noenergy = microAnimationGet("robot-2-walk-noenergy");
  anim_player_jump_noenergy = microAnimationGet("robot-2-jump-noenergy");
  anim_player_solar_charging = microAnimationGet("robot-2-charging");
  anim_shield = microAnimationGet("shield");

  player_entity_id = microECSEntityNew(NULL, NULL);
  assert(player_entity_id != -1);

  CmpAddPosition(player_entity_id, x, y);

  player_body_id = microPhysicsBodyNewCircle(player_entity_id, 0, x, y,
                                           PLAYER_BODY_RADIUS, 1.0, FALSE,
                                           FALSE, 0.0, 0.0);
  microPhysicsBodySetCollisionCallback(player_body_id, PlayerCollide);
  microPhysicsBodySetFilter(player_body_id, COLLISION_GROUP_PLAYER,
                            COLLISION_MASK_PLAYER);
  CmpAddBody(player_entity_id, player_body_id);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  CmpAddSprite(player_entity_id, textureId, 0, 0, 16, 16);
  CmpAddTransform(player_entity_id, PLAYER_TEXTURE_WIDTH, PLAYER_TEXTURE_HEIGHT,
                  PLAYER_TEXTURE_WIDTH / 2.0, PLAYER_TEXTURE_HEIGHT / 2.0, 0.0);
  CmpAddColor(player_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(player_entity_id, LAYER_PLAYER, 1);
  CmpAddAnimation(player_entity_id, anim_player_idle, 1.0, FALSE, FALSE, FALSE);
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
  robot_say_introduce = microResourceGet("robot_introduce");
  robot_say_no = microResourceGet("robot_say_no");
  robot_say_yes = microResourceGet("robot_say_yes");
  robot_jump = microResourceGet("robot_jump");
  robot_footstep = microResourceGet("robot_footstep");
  gun_shot = microResourceGet("gun_shot");
  microSoundSetVolume(gun_shot, 0.2);
  robot_recharging = microResourceGet("robot_recharging");
  robot_alarm = microResourceGet("robot_alarm");
  robot_shield_hit = microResourceGet("robot_shield_hit");
  robot_engine = microResourceGet("robot_engine");

  player_state = PLAYER_STATE_LANDING;
}
