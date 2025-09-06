#include "State.h"
#include "ECS.h"
#include "Graphics.h"
#include "System.h"
#include <SDL2/SDL.h>

static MicroState currentState = {NULL, NULL, NULL, 0.0};
static int stateChangeRequested = 0;
static MicroState nextState = {NULL, NULL, NULL, 0.0};
static double lastBusyTime = 0.0;

void microStateSet(MicroState state)
{
  nextState = state;
  stateChangeRequested = 1;
}

void microStateUpdate(float dt)
{
  uint64_t freq = SDL_GetPerformanceFrequency();
  uint64_t start, end;

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
  start = SDL_GetPerformanceCounter();
  microECSRun(dt);
  microSystemUpdate();
  currentState.update(dt);
  end = SDL_GetPerformanceCounter();
  lastBusyTime = (double)(end - start) / freq;
  microGraphicsDisplay();
  microSystemWindowSwapBuffers();
}

void microStateFree()
{
  if (currentState.free != NULL)
    currentState.free();
  currentState.free = NULL;
  currentState.update = NULL;
  currentState.init = NULL;
}

double microStateGetLastBusyTime()
{
  return lastBusyTime;
}

double microStateGetTime()
{
  return currentState.time;
}
