#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>


#include "Audio.h"
#include "Graphics.h"
#include "ECS.h"

#include "components/CPosition.h"
#include "components/CSprite.h"
#include "components/CShadedCanvas.h"
#include "components/CUpdate.h"
#include "systems/SpriteSystem.h"
#include "systems/ShadedCanvasSystem.h"
#include "systems/UpdateSystem.h"
#include "entities/Space.h"
#include "entities/Planet.h"


int main(int argc, char const *argv[])
{
  microGraphicsInit();
  
  int windowWidth, windowHeight;
  microWindowGetSize(&windowWidth, &windowHeight);

  microViewSet((MicroView){
      .viewportX = 0,
      .viewportY = 0,
      .viewportWidth = windowWidth,
      .viewportHeight = windowHeight,
      .centerX = windowWidth/2.0,
      .centerY = windowHeight/2.0,
      .width = windowWidth,
      .height = windowHeight,
      .rotation = 0,
      .flipY = 0
      });
  microViewApply();

  microECSInit();

  // Register components
  RegisterCPosition();
  RegisterCSprite();
  RegisterCShadedCanvas();
  RegisterCUpdate();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(spriteSystem);

  // Register entities
  SpaceEntityAdd();
  PlanetEntityAdd();
  
  while (1)
  {
    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT) {
        return 0;
      }

      if (event.type == SDL_KEYDOWN)
      {
        if (event.key.keysym.scancode == SDL_SCANCODE_Q)
          return 0;
      }
    }
    
    // float cx, cy;
    // float r;
    // microViewGetCenter(&cx, &cy);
    // r = microViewGetRotation();
    // float dirX = -sin(-r * 3.14159 / 180.0);
    // float dirY = -cos(-r * 3.14159 / 180.0);
    //
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W]) {
    //   cx += dirX;
    //   cy += dirY;
    // }
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S]) {
    //   cx -= dirX;
    //   cy -= dirY;
    // }
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A]) {
    //   cx += dirY;
    //   cy -= dirX;
    // }
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D]) {
    //   cx -= dirY;
    //   cy += dirX;
    // }
    //
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E])
    //   r += 3.14159 / 10.0;
    // if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_R])
    //   r -= 3.14159 / 10.0;
    //
    // microViewSetCenter(cx, cy);
    // microViewSetRotation(r);
    
    float r;
    r = microViewGetRotation();
    if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
      r += 3.14159 / 20.0;
    if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
      r -= 3.14159 / 20.0;
    float dirX = -sin(-r * 3.14159 / 180.0);
    float dirY = -cos(-r * 3.14159 / 180.0);

    float viewWidth, viewHeight;
    microViewGetSize(&viewWidth, &viewHeight);
    float planetX, planetY;
    float planetNormRadius;
    PlanetGetPos(&planetX, &planetY);
    planetNormRadius = PlanetGetRadius();
    float viewX = planetX + dirX * (planetNormRadius * viewHeight/2.0 + viewHeight * 0.1);
    float viewY = planetY + dirY * (planetNormRadius * viewHeight/2.0 + viewHeight * 0.1);
    microViewSetCenter(viewX, viewY);
    microViewSetRotation(r);
    

    microGraphicsClear();
    float deltaTime = 0.016;
    microECSRun(deltaTime);
    microGraphicsDisplay();
    microSwapBuffers();
  }
  
  microECSFree();
  microGraphicsQuit();

  return 0;
}
