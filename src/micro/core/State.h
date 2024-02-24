#ifndef STATE_HPP
#define STATE_HPP

#include <SDL2/SDL_events.h>

typedef struct
{
  void (*init)();
  void (*update)(float dt);
  void (*free)();
} MicroState;

void microStateSet(MicroState state);
void microStateQuit();
void microStateUpdate(float dt);
void microStateFree();

#endif // STATE_HPP
