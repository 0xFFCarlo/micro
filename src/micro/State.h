#ifndef STATE_HPP
#define STATE_HPP

#include <SDL2/SDL_events.h>

typedef struct {
  void(*init)();
  void(*update)(float dt);
  void(*free)();
} MicroState;

extern void microStateSet(MicroState state);
extern void microStateUpdate(float dt);
extern void microStateFree();

#endif // STATE_HPP
