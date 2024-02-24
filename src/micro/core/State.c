#include "State.h"

MicroState currentState = {NULL, NULL, NULL};
int stateChangeRequested = 0;
MicroState nextState = {NULL, NULL, NULL};

void microStateSet(MicroState state)
{
  nextState = state;
  stateChangeRequested = 1;
}

void microStateQuit()
{
  nextState = (MicroState){NULL, NULL, NULL};
  stateChangeRequested = 1;
}

void microStateUpdate(float dt)
{
  if (stateChangeRequested)
  {
    microStateFree();
    currentState = nextState;
    if (currentState.init != NULL)
      currentState.init();
    stateChangeRequested = 0;
  }

  if (currentState.update == NULL)
    return;
  currentState.update(dt);
}

void microStateFree()
{
  if (currentState.free != NULL)
    currentState.free();
  currentState.free = NULL;
  currentState.update = NULL;
  currentState.init = NULL;
}
