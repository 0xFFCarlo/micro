#include "Player.h"
#include "Planet.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../Resources.h"
#include "../Graphics.h"
#include "../ECS.h"
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
  float viewX, viewY;
  float viewWidth, viewHeight;
  float viewAngle;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);
  viewAngle = microViewGetRotation();

  CPosition* position = (CPosition*)microECSEntityGetComponent(player_entity_id, cid_position);
  CVelocity* velocity = (CVelocity*)microECSEntityGetComponent(player_entity_id, cid_velocity);

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  float toPlanetX = planetX - position->x;
  float toPlanetY = planetY - position->y;
  float toPlanetDist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
  toPlanetNormX = toPlanetX / toPlanetDist;
  toPlanetNormY = toPlanetY / toPlanetDist;
  float playerHeight = PlanetGetRadius() * viewHeight / 2.0 + PLAYER_HEIGHT/2.0 - GROUND_HEIGHT;

  // Motion
  velocity->x = 0.0;
  velocity->y = 0.0;
  if (player_state == PLAYER_STATE_WALK) {
    if (player_direction == PLAYER_DIRECTION_RIGHT) {
      velocity->x = toPlanetNormY * 100.0;
      velocity->y = -toPlanetNormX * 100.0;
    } else {
      velocity->x = -toPlanetNormY * 100.0;
      velocity->y = toPlanetNormX * 100.0;
    }
  }

  if (playerJump) {
    velocity->x += toPlanetNormX * 1000.0;
    velocity->y += toPlanetNormY * 1000.0;
    playerJump = 0;
  }
  playerJump = 0;
    
  // Gravity
  if (toPlanetDist > PlanetGetRadius() * viewHeight / 2.0 + PLAYER_HEIGHT/2.0 - GROUND_HEIGHT) {
    velocity->x += toPlanetNormX * 100.0;
    velocity->y += toPlanetNormY * 100.0;
  }

  // Force out of planet
  toPlanetX = planetX - position->x;
  toPlanetY = planetY - position->y;
  toPlanetDist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
  toPlanetNormX = toPlanetX / toPlanetDist;
  toPlanetNormY = toPlanetY / toPlanetDist;
  float planetRadius = PlanetGetRadius() * viewHeight / 2.0;
  float playerRadius = PLAYER_HEIGHT/2.0 - GROUND_HEIGHT;
  float dist = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY);
  if (dist < planetRadius + playerRadius) {
    position->x = planetX - toPlanetNormX * (planetRadius + playerRadius);
    position->y = planetY - toPlanetNormY * (planetRadius + playerRadius);
  }
  
  // if (player_state == PLAYER_STATE_WALK) {
  //   if (player_direction == PLAYER_DIRECTION_RIGHT) {
  //     a += dt * 0.3;
  //   }
  //   else
  //   {
  //     a -= dt * 0.3;
  //   }
  // }
  // position->x = planetX + cos(a) * playerHeight;
  // position->y = planetY + sin(a) * playerHeight;

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

  // Adjust sprite rotation
  CTransform* t = (CTransform*)microECSEntityGetComponent(player_entity_id, cid_transform);
  float outVecX = position->x - planetX;
  float outVecY = position->y - planetY;
  float outVecLen = sqrt(outVecX * outVecX + outVecY * outVecY);
  outVecX /= outVecLen;
  outVecY /= outVecLen;
  playerRotation = atan2(outVecY, outVecX) * 180.0 / M_PI + 90.0;
  t->rotation = playerRotation;
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

  player_entity_id = microECSEntityNew(NULL, NULL, NULL);
  assert(player_entity_id != -1);

  // Position component
  int viewportWidth, viewportHeight;
  microWindowGetSize(&viewportWidth, &viewportHeight);
  CPosition position = {
    .x = viewportWidth/2.0,
    .y = viewportHeight/2.0,
  };
  microECSEntityAddComponent(player_entity_id, cid_position, &position);
  
  // Velocity component
  CVelocity velocity = {
    .x = 0.0,
    .y = 0.0,
  };
  microECSEntityAddComponent(player_entity_id, cid_velocity, &velocity);
  
  // Sprite component
  //int textureId = microResourceLoad("player", "./res/robot.png", "texture");
  int textureId = microTextureLoadFromFile("./res/robot.png");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  CSprite sprite = {
    .textureId = textureId,
    .tx = 0,
    .ty = 0,
    .tw = 16,
    .th = 16,
  };
  microECSEntityAddComponent(player_entity_id, cid_sprite, &sprite);

  // Transform component
  CTransform transform = {
    .width = PLAYER_WIDTH,
    .height = PLAYER_HEIGHT,
    .originX = PLAYER_WIDTH/2.0,
    .originY = PLAYER_HEIGHT/2.0,
    .rotation = 0.0,
  };
  microECSEntityAddComponent(player_entity_id, cid_transform, &transform);

  // Color component
  CColor color = {
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0,
  };
  microECSEntityAddComponent(player_entity_id, cid_color, &color);

  // Layer component
  CLayer layer = {
    .layerId = 3,
  };
  microECSEntityAddComponent(player_entity_id, cid_layer, &layer);

  // Animation component
  CAnimation animation = {
    .animationId = player_walk_l,
    .frameId = 0,
    .timeSinceLastFrame = 0,
  };
  microECSEntityAddComponent(player_entity_id, cid_animation, &animation);

  // Update component
  CUpdate update = {
    .update = playerUpdate,
  };
  microECSEntityAddComponent(player_entity_id, cid_update, &update);

  // Lock on view component
  CLockOnView lockOnView = {
    .followRotation = 1,
  };
  microECSEntityAddComponent(player_entity_id, cid_lock_on_view, &lockOnView);
}
