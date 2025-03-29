#include "Micro.h"
#include "Audio.h"
#include "ECS.h"
#include "Graphics.h"
#include "Physics.h"
#include "../util/debug.h"
#include "../components/Components.h"
#include "../systems/Systems.h"

static bool running = true;

int microInit(MicroState bootState)
{
  if (microAudioInit() != 0)
    return EXIT_FAILURE;
  if (microGraphicsInit() != 0)
    return EXIT_FAILURE;

  // Setup entity component system and register
  if (microECSInit() != 0)
    return EXIT_FAILURE;
  microComponentsRegisterAll();
  microECSAllocateComponents();
  microSystemsUseAll();

  microStateSet(bootState);
  return 0;
}

static int microFree()
{
  microStateFree();
  debug_print("Freeing ECS\n");
  microECSFree();
  debug_print("Free physics\n");
  microPhysicsWorldFreeAll();
  debug_print("Freeing audio\n");
  microAudioDestroy();
  debug_print("Freeing graphics\n");
  microGraphicsQuit();
  debug_print("Done\n");
  return 0;
}

int microUpdate(int max_fps)
{
  float deltaTime = 0.0;
  while (running)
  {
    microStateUpdate(deltaTime);
    deltaTime = microGraphicsDelayToNextFrame(max_fps);
  }
  microFree();
  memory_check_leaks();
  return 0;
}

void microQuit()
{
  running = false;
}
