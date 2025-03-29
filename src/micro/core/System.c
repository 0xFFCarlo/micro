#include "../util/Types.h"
#include "System.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

SDL_GameController *controller = NULL;

int microSystemGetKey(MicroKey key)
{
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  return state[key];
}

void microSystemGetMousePos(int *x, int *y)
{
  SDL_GetMouseState(x, y);
}

void microSystemSetMousePos(int x, int y)
{
  SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), x, y);
}

void microSystemGetWindowSize(int *width, int *height)
{
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), width, height);
}

void microSystemShowCursor(bool show)
{
  SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}

void microSystemFocusWindow()
{
  SDL_RaiseWindow(SDL_GL_GetCurrentWindow());
}

bool microSystemIsGamepadConnected()
{
  return SDL_NumJoysticks() > 0;
}

SDL_GameController *microSystemGetGameController()
{
  if (controller == NULL)
  {
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
      if (SDL_IsGameController(i))
      {
        controller = SDL_GameControllerOpen(i);
        break;
      }
    }
  }
  return controller;
}
