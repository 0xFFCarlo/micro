#ifndef LOGIC_COMPONENTS_H
#define LOGIC_COMPONENTS_H

#include <SDL2/SDL_events.h>

// Update
typedef struct
{
  void (*update)(int, float);
} CUpdate;

int cid_update;
void RegisterCUpdate();
void CmpAddUpdate(int entity_id, void (*update)(int, float));
CUpdate *CmpGetUpdate(int entity_id);

// EventListener
typedef struct
{
  void (*on_event)(int, const SDL_Event *);
} CEventListener;

int cid_event_listener;
void RegisterCEventListener();
void CmpAddEventListener(int entity_id,
                                void (*on_event)(int, const SDL_Event *));
CEventListener *CmpGetEventListener(int entity_id);

typedef struct
{
  float lifetime;
} CLifetime;

int cid_lifetime;
void RegisterCLifetime();
void CmpAddLifetime(int entity_id, float lifetime);
CLifetime *CmpGetLifetime(int entity_id);

void RegisterLogicComponents();

#endif /* end of include guard: LOGIC_COMPONENTS_H */
