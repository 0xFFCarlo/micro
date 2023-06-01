#include "Player.h"
#include "Planet.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/CustomComponents.h"
#include "../Resources.h"
#include "../Graphics.h"
#include "../ECS.h"
#include "../Physics.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define PLAYER_STATE_IDLE 0
#define PLAYER_STATE_WALK 1

#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1

#define PLAYER_WIDTH 64
#define PLAYER_HEIGHT 64
#define GROUND_HEIGHT 10

int player_entity_id = -1;
int player_idle = -1;
int player_state = 0;
int player_direction = 0;
  
int player_idle_r = -1;
int player_idle_l = -1;
int player_walk_r = -1;
int player_walk_l = -1;
  
float toPlanetNormX = 0.0;
float toPlanetNormY = 0.0;

float playerRotation = 0.0;
int playerJump = 0;

int player_body_id = -1;

void PlayerMove(int direction)
{
  if (direction == PLAYER_DIRECTION_RIGHT) {
    player_direction = PLAYER_DIRECTION_RIGHT;
    player_state = PLAYER_STATE_WALK;
  } else {
    player_direction = PLAYER_DIRECTION_LEFT;
    player_state = PLAYER_STATE_WALK;
  }
}

void PlayerJump()
{
  playerJump = 1;
}

void playerUpdate(int entityId, float dt)
{
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

  CPosition* position = (CPosition*)microECSEntityGetComponent(player_entity_id, cid_position);

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  float toPlanetX = planetX - position->x;
  float toPlanetY = planetY - position->y;
  float toPlanetDist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
  toPlanetNormX = toPlanetX / toPlanetDist;
  toPlanetNormY = toPlanetY / toPlanetDist;
  float playerHeight = PlanetGetRadius() * viewHeight / 2.0 + PLAYER_HEIGHT/2.0 - GROUND_HEIGHT;
  
  // Gravity
  float forceX = (100000.0 * toPlanetX) / (toPlanetDist * toPlanetDist);
  float forceY = (100000.0 * toPlanetY) / (toPlanetDist * toPlanetDist);

  // Motion on planet 
  if (toPlanetDist <= playerHeight + 1.0) {

    // float px, py;
    // microPhysicsBodyGetPosition(player_body_id, &px, &py);
    float vx, vy;
    microPhysicsBodyGetVelocity(player_body_id, &vx, &vy);
    
    // Motion
    if (player_state == PLAYER_STATE_WALK) {
      if (player_direction == PLAYER_DIRECTION_RIGHT) {
        forceX += toPlanetNormY * 150000.0 * dt;
        forceY += -toPlanetNormX * 150000.0 * dt;
      } else {
        forceX += -toPlanetNormY * 150000.0 * dt;
        forceY += toPlanetNormX * 150000.0 * dt;
      }
    }

    if (playerJump) {
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
  CAnimation* animation = (CAnimation*)microECSEntityGetComponent(player_entity_id, cid_animation);
  if (player_state == PLAYER_STATE_WALK) {
    if (player_direction == PLAYER_DIRECTION_RIGHT) {
      if (animation->animationId != player_walk_r)
        animation->frameId = 0;
      animation->animationId = player_walk_r;
    } else {
      if (animation->animationId != player_walk_l)
        animation->frameId = 0;
      animation->animationId = player_walk_l;
    }
  }
  else
  {
    if (player_direction == PLAYER_DIRECTION_RIGHT) {
      if (animation->animationId != player_idle_r)
        animation->frameId = 0;
      animation->animationId = player_idle_r;
    } else {
      if (animation->animationId != player_idle_l)
        animation->frameId = 0;
      animation->animationId = player_idle_l;
    }
  }
  player_state = PLAYER_STATE_IDLE;
  
  CPlanetaryAlignment* pa = (CPlanetaryAlignment*)microECSEntityGetComponent(player_entity_id, cid_planetary_alignment);
  pa->planet_x = planetX;
  pa->planet_y = planetY;
}

void PlayerGetPos(float *x, float *y)
{
  CPosition* position = (CPosition*)microECSEntityGetComponent(player_entity_id, cid_position);
  *x = position->x;
  *y = position->y;
}

float PlayerGetRotation()
{
  return playerRotation;
}

void PlayerEntityAdd()
{
  player_idle_r = microAnimationCreate("player_idle_r", 0, 0, 16, 16, 2, 1.0, 0, 0);
  player_idle_l = microAnimationCreate("player_idle_r", 0, 0, 16, 16, 2, 1.0, 1, 0);
  player_walk_r = microAnimationCreate("player_walk_r", 0, 16, 16, 16, 4, 0.3, 0, 0);
  player_walk_l = microAnimationCreate("player_walk_l", 0, 16, 16, 16, 4, 0.3, 1, 0);

  player_entity_id = microECSEntityNew(NULL, NULL); 
  assert(player_entity_id != -1);

  // Position component
  int viewportWidth, viewportHeight;
  microWindowGetSize(&viewportWidth, &viewportHeight);
  microECSEntityAddComponent(player_entity_id, cid_position, &(CPosition){
    .x = viewportWidth/2.0,
    .y = viewportHeight/2.0 - 100.0,
  });
  
  // Body component
  player_body_id = microPhysicsBodyNewCircle(0, viewportWidth/2.0, viewportHeight/2.0 - 100.0, PLAYER_WIDTH/2.0, 1.0, 0, 1.0, 0.0, 1.0);
  microECSEntityAddComponent(player_entity_id, cid_body, &(CBody) {
      .body_id = player_body_id,
  });
  
  // Sprite component
  //int textureId = microResourceLoad("player", "./res/robot.png", "texture");
  int textureId = microTextureLoadFromFile("./res/robot.png");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  microECSEntityAddComponent(player_entity_id, cid_sprite, &(CSprite){
    .textureId = textureId,
    .tx = 0,
    .ty = 0,
    .tw = 16,
    .th = 16,
  });

  // Transform component
  microECSEntityAddComponent(player_entity_id, cid_transform, &(CTransform){
    .width = PLAYER_WIDTH,
    .height = PLAYER_HEIGHT,
    .originX = PLAYER_WIDTH/2.0,
    .originY = PLAYER_HEIGHT/2.0,
    .rotation = 0.0,
  });

  // Color component
  microECSEntityAddComponent(player_entity_id, cid_color, &(CColor){
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0,
  });

  // Layer component
  microECSEntityAddComponent(player_entity_id, cid_drawable, &(CDrawable){
    .layerId = 4,
  });

  // Animation component
  microECSEntityAddComponent(player_entity_id, cid_animation, &(CAnimation){
    .animationId = player_idle_l,
    .frameId = 0,
    .timeSinceLastFrame = 0,
  });

  // Update component
  microECSEntityAddComponent(player_entity_id, cid_update, &(CUpdate){
    .update = playerUpdate,
  });

  // Lock on view component
  microECSEntityAddComponent(player_entity_id, cid_lock_on_view, &(CLockOnView){
    .followRotation = 1,
  });
  
  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  microECSEntityAddComponent(player_entity_id, cid_planetary_alignment, &(CPlanetaryAlignment) {
    .planet_x = planetX,
    .planet_y = planetY,
  });
}
