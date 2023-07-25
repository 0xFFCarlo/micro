#ifndef PLAYER_H
#define PLAYER_H

extern void PlayerEntityAdd();
extern void PlayerGetPos(float *x, float *y);
extern float PlayerGetRotation();
extern void PlayerMove(int direction);
extern void PlayerJump();
extern int PlayerGetHealth();
extern int PlayerGetMaxHealth();
extern void PlayerGetSize(float *width, float *height);

#endif /* end of include guard: PLAYER_H */
