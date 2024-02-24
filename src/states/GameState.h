#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "../micro/core/State.h"

MicroState gameStateGet();
void gameStateInit();
void gameStateUpdate(float dt);
void gameStateFree();

#endif /* end of include guard: GAME_STATE_H */
