#ifndef STATE_HPP
#define STATE_HPP

#include <SDL2/SDL_events.h>

typedef struct {
  void(*init)();
  void(*handleEvent)(SDL_Event event);
  void(*update)(float dt);
  void(*draw)();
  void(*free)();
} MicroState;

extern void microStateSet(MicroState state);
extern void microStateHandleEvent(SDL_Event event);
extern void microStateUpdate(float dt);
extern void microStateDraw();
extern void microStateFree();

#endif // STATE_HPP
