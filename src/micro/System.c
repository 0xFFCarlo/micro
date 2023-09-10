#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

void microSystemGetMousePos(int *x, int *y)
{
  SDL_GetMouseState(x, y);
}

void microSystemGetWindowSize(int *width, int *height)
{
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), width, height);
}
