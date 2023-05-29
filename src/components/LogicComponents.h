#ifndef LOGIC_COMPONENTS_H
#define LOGIC_COMPONENTS_H

#include <SDL2/SDL_events.h>

// Update
typedef struct
{
  void (*update)(int, float);
} CUpdate;

extern int cid_update;
extern void RegisterCUpdate();

// EventListener
typedef struct
{
  void (*on_event)(int, SDL_Event);
} CEventListener;

extern int cid_event_listener;
extern void RegisterCEventListener();

extern void RegisterLogicComponents();

#endif /* end of include guard: LOGIC_COMPONENTS_H */
