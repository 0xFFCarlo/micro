#include "System.h"
#include "../util/Types.h"
#include "../util/debug.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

static SDL_GameController *controller = NULL;
static SDL_GLContext *context = NULL;
static SDL_Window *window = NULL;
static const uint8_t *currKeys = NULL;
static uint8_t *prevKeys = NULL;
static int keyCount = 0;
static uint32_t currMouseState = 0;
static uint32_t prevMouseState = 0;

int microSystemInit()
{
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    debug_print("Failed to initialize th SDL2 library\n");
    return -1;
  }

  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  const unsigned int screenWidth = DM.w;
  const unsigned int screenHeight = DM.h;

  // create window and opengl context
  window = SDL_CreateWindow("micro", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                            SDL_WINDOW_OPENGL |
                              SDL_WINDOW_SHOWN); //| SDL_WINDOW_INPUT_FOCUS);

  // Set fullscreen
  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
  // SDL_SetWindowFullscreen(window, 0);  // Switches to windowed mode
  // SDL_MaximizeWindow(window);

  if (!window)
  {
    debug_print("Failed to create the window\n");
    return -1;
  }

  // Set OpenGL version
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // Initilize Opengl
  context = SDL_GL_CreateContext(window);
  if (context == NULL)
  {
    debug_print("Couldn't create OpenGL context\n");
    return -1;
  }

  // Set VSYNC, try adaptive first and if not supported, use normal vsync
  if (SDL_GL_SetSwapInterval(-1) == -1)
  {
    if (SDL_GL_SetSwapInterval(1) == -1)
      printf("Unable to set VSYNC! SDL Error: %s\n", SDL_GetError());
  }
  // SDL_GL_SetSwapInterval(0);

  // Initialize keyboard state arrays
  currKeys = SDL_GetKeyboardState(&keyCount);
  prevKeys = malloc(keyCount);
  memset(prevKeys, 0, keyCount);

  return 0;
}

int microSystemFree()
{
  free(prevKeys);
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

void microSystemUpdate()
{

  SDL_PumpEvents();
  memcpy(prevKeys, currKeys, keyCount);
  prevMouseState = currMouseState;
  currKeys = SDL_GetKeyboardState(NULL);
  currMouseState = SDL_GetMouseState(NULL, NULL);
}

void microSystemWindowSwapBuffers()
{
  SDL_GL_SwapWindow(window);
}

int microSystemKeyIsDown(MicroKey scancode)
{
  return currKeys[scancode];
}

int microSystemKeyIsPress(MicroKey scancode)
{
  return currKeys[scancode] && !prevKeys[scancode];
}

int microSystemKeyIsReleased(MicroKey scancode)
{
  return !currKeys[scancode] && prevKeys[scancode];
}

void microSystemGetMousePos(int *x, int *y)
{
  SDL_GetMouseState(x, y);
}

void microSystemGetRelativeMousePos(int *x, int *y)
{
  SDL_GetRelativeMouseState(x, y);
}

void microSystemSetMousePos(int x, int y)
{
  SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), x, y);
}

bool microSystemMouseIsDown(MicroMouseButton button)
{
  return (currMouseState & SDL_BUTTON(button)) != 0;
}

bool microSystemMouseIsPress(MicroMouseButton button)
{
  return (currMouseState & SDL_BUTTON(button)) != 0 &&
         (prevMouseState & SDL_BUTTON(button)) == 0;
}

bool microSystemMouseIsReleased(MicroMouseButton button)
{
  return (currMouseState & SDL_BUTTON(button)) == 0 &&
         (prevMouseState & SDL_BUTTON(button)) != 0;
}

void microSystemSetRelativeMouseMode(bool enabled)
{
  if (enabled == false)
  {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_SetWindowGrab(window, SDL_FALSE);
    return;
  }
  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_SetWindowGrab(window, SDL_TRUE);
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
  SDL_RaiseWindow(window);
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
