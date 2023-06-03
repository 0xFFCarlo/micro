#include "State.h"
#include <stdlib.h>

MicroState currentState = {NULL, NULL, NULL};

void microStateSet(MicroState state)
{
  microStateFree();
  currentState = state;
  if (currentState.init != NULL)
    currentState.init();
}


void microStateUpdate(float dt)
{
  if (currentState.update == NULL) return;
  currentState.update(dt);
}

void microStateFree()
{
  if (currentState.free != NULL) currentState.free();
  currentState.free = NULL;
  currentState.update = NULL;
  currentState.init = NULL;
}
