#include <SDL2/SDL.h>
#include "./micro/Graphics.h"
#include "./micro/ECS.h"
#include "./micro/State.h"

#include "states/GameState.h"
#include <string.h>

int main(int argc, char const *argv[])
{
  microGraphicsInit();
  microECSInit();
  microStateSet(gameStateGet());

  float deltaTime = 0;
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
