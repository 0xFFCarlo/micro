#include "LogicComponents.h"
#include "../core/ECS.h"
#include <stdlib.h>

int cid_update = -1;
int cid_event_listener = -1;
int cid_lifetime = -1;

void RegisterCUpdate()
{
  cid_update = microECSComponentRegister(sizeof(CUpdate));
}

void CmpAddUpdate(int entity_id, void (*update)(int, float))
{
  microECSEntityAddComponent(entity_id, cid_update,
                             &(CUpdate){
                               .update = update,
                             });
}

CUpdate *CmpGetUpdate(int entity_id)
{
  return (CUpdate *)microECSEntityGetComponent(entity_id, cid_update);
}

void RegisterCEventListener()
{
  cid_event_listener = microECSComponentRegister(sizeof(CEventListener));
}

void CmpAddEventListener(int entity_id,
                         void (*on_event)(int, const SDL_Event *))
{
  microECSEntityAddComponent(entity_id, cid_event_listener,
                             &(CEventListener){
                               .on_event = on_event,
                             });
}

CEventListener *CmpGetEventListener(int entity_id)
{
  return (CEventListener *)microECSEntityGetComponent(entity_id,
                                                      cid_event_listener);
}

void RegisterCLifetime()
{
  cid_lifetime = microECSComponentRegister(sizeof(CLifetime));
}

void CmpAddLifetime(int entity_id, float lifetime)
{
  microECSEntityAddComponent(entity_id, cid_lifetime,
                             &(CLifetime){
                               .lifetime = lifetime,
                             });
}

CLifetime *CmpGetLifetime(int entity_id)
{
  return (CLifetime *)microECSEntityGetComponent(entity_id, cid_lifetime);
}

void RegisterLogicComponents()
{
  RegisterCUpdate();
  RegisterCEventListener();
  RegisterCLifetime();
}
