#ifndef STATE_HPP
#define STATE_HPP

#include <SDL2/SDL_events.h>

typedef struct
{
  void (*init)();
  void (*update)(float dt);
  void (*free)();
  double time;
} MicroState;

void microStateSet(MicroState state);
void microStateUpdate(float dt);
void microStateFree();
double microStateGetTime();

#endif // STATE_HPP
