#include "State.h"
#include "Graphics.h"
#include "ECS.h"

MicroState currentState = {NULL, NULL, NULL, 0.0};
int stateChangeRequested = 0;
MicroState nextState = {NULL, NULL, NULL, 0.0};

void microStateSet(MicroState state)
{
  nextState = state;
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

  currentState.time += dt;
  microGraphicsClear();
  currentState.update(dt);
  microECSRun(dt);
  microGraphicsDisplay();
  microSwapBuffers();
}

void microStateFree()
{
  if (currentState.free != NULL)
    currentState.free();
  currentState.free = NULL;
  currentState.update = NULL;
  currentState.init = NULL;
}

double microStateGetTime()
{
  return currentState.time;
}
