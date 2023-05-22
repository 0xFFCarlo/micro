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
#include "components/CAnimation.h"
#include "components/CVelocity.h"
#include "components/CLockOnView.h"
#include "systems/SpriteSystem.h"
#include "systems/ShadedCanvasSystem.h"
#include "systems/UpdateSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/VelocitySystem.h"
#include "systems/LockOnViewSystem.h"
#include "entities/Space.h"
#include "entities/Planet.h"
#include "entities/Player.h"


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
  RegisterCAnimation();
  RegisterCVelocity();
  RegisterCLockOnView();
  microECSAllocateComponents();

  // Register systems
  microECSSystemAdd(updateSystem);
  microECSSystemAdd(lockOnViewSystem);
  microECSSystemAdd(velocitySystem);
  microECSSystemAdd(shadedCanvasSystem);
  microECSSystemAdd(spriteSystem);
  microECSSystemAdd(animationSystem);

  // Register entities
  PlayerEntityAdd();
  SpaceEntityAdd();
  PlanetEntityAdd();

  int font = microFontLoadFromFile("./res/FiraCode-Medium.ttf", 20, MICRO_FILTER_NEAREST);
  // int font = microFontLoadFromFile("./res/calibri.ttf", 32);
  
  float deltaTime = 0.016;
  float lastTime = 0.0;
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
    
    microGraphicsClear();
    
    if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D])
      PlayerMove(0);
    else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A])
      PlayerMove(1);
    else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W])
      PlayerJump();

    
    microECSRun(deltaTime);
    
    // microGraphicsDrawRect(
    //     microFontGetTextureId(font), 0, 0, 512, 512, 100, 100, 512, 512, 1.0, 1.0, 1.0, 1.0);
    
    microGraphicsDrawText(font, "Hello World!\nI am Carlo! <3", 100, 100, 1.0, 1.0, 1.0, 1.0);

    microGraphicsDisplay();
    microSwapBuffers();

    //print view position and angle
    // float viewX, viewY, viewAngle;
    // microViewGetCenter(&viewX, &viewY);
    // viewAngle = microViewGetRotation();
    // printf("View: (%f, %f) %f\n", viewX, viewY, viewAngle);


    deltaTime = (SDL_GetTicks() - lastTime) / 1000.0;
    lastTime = SDL_GetTicks();
  }
  
  microECSFree();
  microGraphicsQuit();

  return 0;
}
