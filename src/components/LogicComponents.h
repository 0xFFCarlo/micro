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
extern void CmpAddUpdate(int entity_id, void (*update)(int, float));
extern CUpdate *CmpGetUpdate(int entity_id);

// EventListener
typedef struct
{
  void (*on_event)(int, const SDL_Event *);
} CEventListener;

extern int cid_event_listener;
extern void RegisterCEventListener();
extern void CmpAddEventListener(int entity_id,
                                void (*on_event)(int, const SDL_Event *));
extern CEventListener *CmpGetEventListener(int entity_id);

typedef struct
{
  float lifetime;
} CLifetime;

extern int cid_lifetime;
extern void RegisterCLifetime();
extern void CmpAddLifetime(int entity_id, float lifetime);
extern CLifetime *CmpGetLifetime(int entity_id);

extern void RegisterLogicComponents();

#endif /* end of include guard: LOGIC_COMPONENTS_H */
