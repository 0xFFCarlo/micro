#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>


#include "Audio.h"
#include "Graphics.h"
#include "ECS.h"

#include "components/CPosition.h"
#include "components/CSprite.h"
#include "systems/SpriteSystem.h"
#include "entities/Planet.h"


int main(int argc, char const *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize th SDL2 library\n");
    return -1;
  }
  //create window and opengl context
  SDL_Window *window = SDL_CreateWindow("Test Window",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800, 600,
      SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
      );

  if (!window) {
    printf("Failed to create the window\n");
    return -1;
  }

  //get window size
  int windowWidth, windowHeight;
  SDL_GetWindowSize(window, &windowWidth, &windowHeight);

  //intialize micro graphics
  printf("Initializing graphics...\n");
  microGraphicsInit(window);
  printf("Done\n");

  //initialize view
  microViewSet(0, 0, windowWidth, windowHeight,
      windowWidth/2.0, windowHeight/2.0,
      windowWidth, windowHeight, 0, 0);
  microViewUpdate();

  microECSInit();

  // Register components
  RegisterCPosition();
  RegisterCSprite();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(spriteSystem);

  // Register entities
  PlanetEntityAdd();

  while (1)
  {
    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        return 0;

      if (event.type == SDL_KEYDOWN)
      {
        if (event.key.keysym.scancode == SDL_SCANCODE_Q)
          return 0;
      }
    }

    microGraphicsClear();
    
    float deltaTime = 0.016;
    microECSRun(deltaTime);

    microGraphicsDisplay();


    SDL_GL_SwapWindow(window);
    SDL_Delay(16);
  }

  microGraphicsQuit();
  SDL_DestroyWindow( window );
  SDL_Quit();

  return 0;
}
