#ifndef SYSTEM_H
#define SYSTEM_H

#include "../util/Types.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>

typedef enum
{
  MICRO_MOUSE_BUTTON_LEFT,
  MICRO_MOUSE_BUTTON_MIDDLE,
  MICRO_MOUSE_BUTTON_RIGHT,
  MICRO_MOUSE_BUTTON_COUNT,
} MicroMouseButton;

int microSystemInit();
int microSystemFree();
void microSystemUpdate();
int microSystemKeyIsDown(SDL_Scancode scancode);
int microSystemKeyIsPress(SDL_Scancode scancode);
int microSystemKeyIsReleased(SDL_Scancode scancode);
SDL_Keymod microSystemGetKeyModState();
bool microSystemMouseIsDown(MicroMouseButton button);
bool microSystemMouseIsPress(MicroMouseButton button);
bool microSystemMouseIsReleased(MicroMouseButton button);
void microSystemGetMousePos(int *x, int *y);
void microSystemGetRelativeMousePos(int *x, int *y);
void microSystemSetMousePos(int x, int y);
void microSystemSetRelativeMouseMode(bool enabled);
void microSystemGetWindowSize(int *width, int *height);
void microSystemShowCursor(bool show);
void microSystemFocusWindow();
void microSystemWindowSwapBuffers();
bool microSystemIsGamepadConnected();
// SDL_GameController *microSystemGetGameController();

#endif /* end of include guard: SYSTEM_H */
