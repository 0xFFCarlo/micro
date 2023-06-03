#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "../micro/State.h"

extern MicroState gameStateGet();
extern void gameStateInit();
extern void gameStateUpdate(float dt);
extern void gameStateFree();

#endif /* end of include guard: GAME_STATE_H */
