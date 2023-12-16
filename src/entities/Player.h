#ifndef PLAYER_H
#define PLAYER_H

#include "../micro/Types.h"

typedef struct PlayerData {
  float movementSpeed;
  float jumpPower;
  float maxHealth;
  float shootCooldown;
} PlayerData;

typedef enum PlayerState {
  PLAYER_STATE_IDLE,
  PLAYER_STATE_WALK,
  PLAYER_STATE_JUMP,
  PLAYER_STATE_SOLAR_CHARGING,
  PLAYER_STATE_LANDING,
  PLAYER_STATE_DEPARTING,
  PLAYER_STATE_WARP_DRIVE,
  PLAYER_STATE_DEAD,
} PlayerState;

extern void PlayerEntityAdd(const float x, const float y);
extern void PlayerGetPos(float *x, float *y);
extern float PlayerGetRotation();
extern void PlayerMove(int direction);
extern void PlayerJump();
extern float PlayerGetHealth();
extern float PlayerGetMaxHealth();
extern bool PlayerIsAlive();
extern void PlayerGetSize(float *width, float *height);
extern void PlayerHit(float damage);
extern PlayerState PlayerGetState();

#endif /* end of include guard: PLAYER_H */
