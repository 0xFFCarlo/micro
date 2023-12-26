#include "./micro/ECS.h"
#include "./micro/Graphics.h"
#include "./micro/State.h"
#include <SDL2/SDL.h>

#include "states/GameState.h"
#include "states/TestState.h"
#include <string.h>

int main()
{
  srand(time(NULL));

  microGraphicsInit();
  microECSInit();
  // microStateSet(testStateGet());
  microStateSet(gameStateGet());

  float deltaTime = 0.0;
  while (1)
  {
    microStateUpdate(deltaTime);
    deltaTime = microGraphicsDelayToNextFrame(70);
  }

  microStateFree();
  microECSFree();
  microGraphicsQuit();

  return 0;
}
