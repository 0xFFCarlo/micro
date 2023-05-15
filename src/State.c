#include "State.h"
#include <stdlib.h>

MicroState currentState = {NULL, NULL, NULL, NULL, NULL};

void microStateSet(MicroState state)
{
  microStateFree();
  currentState = state;
  if (currentState.init != NULL)
    currentState.init();
}


void microStateHandleEvent(SDL_Event event)
{
  if (currentState.handleEvent == NULL) return;
  currentState.handleEvent(event);
}

void microStateUpdate(float dt)
{
  if (currentState.update == NULL) return;
  currentState.update(dt);
}

void microStateDraw()
{
  if (currentState.draw == NULL) return;
  currentState.draw();
}

void microStateFree()
{
  if (currentState.free != NULL) currentState.free();
  currentState.free = NULL;
  currentState.update = NULL;
  currentState.draw = NULL;
  currentState.init = NULL;
  currentState.handleEvent = NULL;
}
