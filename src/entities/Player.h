#ifndef PLAYER_H
#define PLAYER_H

extern void PlayerEntityAdd();
extern void PlayerGetPos(float *x, float *y);
extern float PlayerGetRotation();
extern void PlayerMove(int direction);
extern void PlayerJump();
extern float PlayerGetHealth();
extern float PlayerGetMaxHealth();
extern void PlayerGetSize(float *width, float *height);
extern void PlayerShieldHit(float damage);

#endif /* end of include guard: PLAYER_H */
