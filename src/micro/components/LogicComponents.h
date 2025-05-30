#ifndef LOGIC_COMPONENTS_H
#define LOGIC_COMPONENTS_H

#include <SDL2/SDL_events.h>

// Update
typedef struct
{
  void (*update)(int, float);
} CUpdate;

extern int cid_update;
void RegisterCUpdate();
void CmpAddUpdate(int entity_id, void (*update)(int, float));
CUpdate *CmpGetUpdate(int entity_id);

// Scripted Update
typedef struct
{
} CScriptedUpdate;

extern int cid_scripted_update;
void RegisterCScriptedUpdate();
void CmpAddScriptedUpdate(int entity_id);
CScriptedUpdate *CmpGetScriptedUpdate(int entity_id);
typedef void (*UpdateHandlerType)(int* eids, int count, float dt);
void CmpSetScriptedUpdateCb(UpdateHandlerType cb);

// EventListener
typedef struct
{
  void (*on_event)(int, const SDL_Event *);
} CEventListener;

extern int cid_event_listener;
void RegisterCEventListener();
void CmpAddEventListener(int entity_id,
                         void (*on_event)(int, const SDL_Event *));
CEventListener *CmpGetEventListener(int entity_id);

typedef struct
{
  float lifetime;
  float max_lifetime;
} CLifetime;

extern int cid_lifetime;
void RegisterCLifetime();
void CmpAddLifetime(int entity_id, float lifetime);
CLifetime *CmpGetLifetime(int entity_id);

typedef struct
{
  int category;
} CEntityCategory;

extern int cid_entity_category;
void RegisterCEntityCategory();
void CmpAddEntityCategory(int entity_id, int category);
CEntityCategory *CmpGetEntityCategory(int entity_id);

typedef struct
{
  const char *name;
} CName;

extern int cid_name;
void RegisterCName();
void CmpAddName(int entity_id, const char *name);
CName *CmpGetName(int entity_id);

typedef struct CHealth
{
  unsigned int max;
  unsigned int current;
} CHealth;
extern int cid_health;
void RegisterCHealth();
void CmpAddHealth(int eid, unsigned int max, unsigned int current);
CHealth *CmpGetHealth(int eid);

typedef struct
{
  const char *action_name;
  const char *description;
  const uint32_t cost;
} CInteractiveDesc;

extern int cid_interactive_desc;
void RegisterCInteractiveDesc();
void CmpAddInteractiveDesc(int entity_id, const char *action_name,
                           const char *description, const uint32_t cost);
CInteractiveDesc *CmpGetInteractiveDesc(int entity_id);

void RegisterLogicComponents();

#endif /* end of include guard: LOGIC_COMPONENTS_H */
