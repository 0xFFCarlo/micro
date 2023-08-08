#include "Player.h"
#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PLAYER_STATE_IDLE 0
#define PLAYER_STATE_WALK 1

#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1

#define PLAYER_TEXTURE_WIDTH 64
#define PLAYER_TEXTURE_HEIGHT 64
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
  if (player_state == PLAYER_STATE_WALK)
  {
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
    animation->framesDuration = 0.55;
  }
  else
  {
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
  anim_player_idle = microAnimationGet("player-idle");
  anim_player_walk = microAnimationGet("player-walk");
  anim_player_jump = microAnimationGet("player-jump");
  // anim_player_idle = microAnimationGet("redsauron-small-idle");
  // anim_player_walk = microAnimationGet("redsauron-small-walk");

  player_entity_id = microECSEntityNew(NULL, NULL);
  assert(player_entity_id != -1);

  // Position component
  int viewportWidth, viewportHeight;
  microWindowGetSize(&viewportWidth, &viewportHeight);
  int x, y;
  x = viewportWidth / 2.0;
  y = viewportHeight / 2.0 - 100.0;
  microECSEntityAddComponent(player_entity_id, cid_position,
                             &(CPosition){
                               .x = x,
                               .y = y,
                             });

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

  microECSEntityAddComponent(player_entity_id, cid_body,
                             &(CBody){
                               .body_id = player_body_id,
                             });

  // Sprite component
  // int textureId = microTextureLoadFromFile("./res/robot.png");
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  microECSEntityAddComponent(player_entity_id, cid_sprite,
                             &(CSprite){
                               .textureId = textureId,
                               .tx = 0,
                               .ty = 0,
                               .tw = 16,
                               .th = 16,
                             });

  // Transform component
  microECSEntityAddComponent(player_entity_id, cid_transform,
                             &(CTransform){
                               .width = PLAYER_TEXTURE_WIDTH,
                               .height = PLAYER_TEXTURE_HEIGHT,
                               .originX = PLAYER_TEXTURE_WIDTH / 2.0,
                               .originY = PLAYER_TEXTURE_HEIGHT / 2.0,
                               .rotation = 0.0,
                             });

  // Color component
  microECSEntityAddComponent(player_entity_id, cid_color,
                             &(CColor){
                               .r = 1.0,
                               .g = 1.0,
                               .b = 1.0,
                               .a = 1.0,
                             });

  // Layer component
  microECSEntityAddComponent(player_entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = 4,
                               .visible = 1,
                             });

  // Animation component
  microECSEntityAddComponent(player_entity_id, cid_animation,
                             &(CAnimation){
                               .animationId = anim_player_idle,
                               .frameId = 0,
                               .timeSinceLastFrame = 0,
                               .framesDuration = 1.0,
                               .flipX = 0,
                               .flipY = 0,
                             });

  // Update component
  microECSEntityAddComponent(player_entity_id, cid_update,
                             &(CUpdate){
                               .update = playerUpdate,
                             });

  // Lock on view component
  microECSEntityAddComponent(player_entity_id, cid_lock_on_view,
                             &(CLockOnView){
                               .followRotation = 1,
                             });

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  microECSEntityAddComponent(player_entity_id, cid_planetary_alignment,
                             &(CPlanetaryAlignment){
                               .planet_x = planetX,
                               .planet_y = planetY,
                             });

  // Health
  microECSEntityAddComponent(player_entity_id, cid_health,
                             &(CHealth){
                               .health = 11,
                               .maxHealth = 16,
                             });

  // Gravity component
  // microECSEntityAddComponent(player_entity_id, cid_gravity, &(CGravity) {
  //   .x = planetX,
  //   .y = planetY,
  // });
}
