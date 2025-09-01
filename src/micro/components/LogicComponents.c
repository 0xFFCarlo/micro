#include "LogicComponents.h"
#include "../core/ECS.h"
#include <assert.h>
#include <stdlib.h>

extern void (*scripted_update_callback)(int *eids, int count, float dt);

int cid_update = -1;
int cid_scripted_update = -1;
int cid_event_listener = -1;
int cid_lifetime = -1;
int cid_entity_category = -1;
int cid_name = -1;
int cid_interactive_desc = -1;
int cid_health = -1;

void RegisterCUpdate()
{
  cid_update = microECSComponentRegister(sizeof(CUpdate), NULL);
}

void CmpAddUpdate(int entity_id, void (*update)(int, float))
{
  assert(cid_update != -1);
  microECSEntityAddComponent(entity_id, cid_update,
                             &(CUpdate){
                               .update = update,
                             });
}

CUpdate *CmpGetUpdate(int entity_id)
{
  return (CUpdate *)microECSEntityGetComponent(entity_id, cid_update);
}

void RegisterCScriptedUpdate()
{
  cid_scripted_update = microECSComponentRegister(sizeof(CScriptedUpdate),
                                                   NULL);
}

void CmpAddScriptedUpdate(int entity_id)
{
  assert(cid_scripted_update != -1);
  microECSEntityAddComponent(entity_id, cid_scripted_update,
                             &(CScriptedUpdate){});
}

CScriptedUpdate *CmpGetScriptedUpdate(int entity_id)
{
  return (CScriptedUpdate *)microECSEntityGetComponent(entity_id,
                                                       cid_scripted_update);
}

void CmpSetScriptedUpdateCb(UpdateHandlerType update_callback)
{
  scripted_update_callback = update_callback;
}

void RegisterCEventListener()
{
  cid_event_listener = microECSComponentRegister(sizeof(CEventListener), NULL);
}

void CmpAddEventListener(int entity_id,
                         void (*on_event)(int, const void *))
{
  assert(cid_event_listener != -1);
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
  cid_lifetime = microECSComponentRegister(sizeof(CLifetime), NULL);
}

void CmpAddLifetime(int entity_id, float lifetime)
{
  assert(cid_lifetime != -1);
  microECSEntityAddComponent(entity_id, cid_lifetime,
                             &(CLifetime){
                               .lifetime = lifetime,
                               .max_lifetime = lifetime,
                             });
}

CLifetime *CmpGetLifetime(int entity_id)
{
  return (CLifetime *)microECSEntityGetComponent(entity_id, cid_lifetime);
}

void RegisterCEntityCategory()
{
  cid_entity_category = microECSComponentRegister(sizeof(CEntityCategory),
                                                  NULL);
}

void CmpAddEntityCategory(int entity_id, int category)
{
  assert(cid_entity_category != -1);
  microECSEntityAddComponent(entity_id, cid_entity_category,
                             &(CEntityCategory){
                               .category = category,
                             });
}

CEntityCategory *CmpGetEntityCategory(int entity_id)
{
  return (CEntityCategory *)microECSEntityGetComponent(entity_id,
                                                       cid_entity_category);
}

void RegisterCName()
{
  cid_name = microECSComponentRegister(sizeof(CName), NULL);
}

void CmpAddName(int entity_id, const char *name)
{
  assert(cid_name != -1);
  microECSEntityAddComponent(entity_id, cid_name,
                             &(CName){
                               .name = name,
                             });
}

CName *CmpGetName(int entity_id)
{
  return (CName *)microECSEntityGetComponent(entity_id, cid_name);
}

void RegisterCHealth()
{
  cid_health = microECSComponentRegister(sizeof(CHealth), NULL);
}

void CmpAddHealth(int eid, unsigned int max, unsigned int current)
{
  assert(cid_health != -1);
  microECSEntityAddComponent(eid, cid_health,
                             &(CHealth){.max = max, .current = current});
}

CHealth *CmpGetHealth(int eid)
{
  return microECSEntityGetComponent(eid, cid_health);
}

void RegisterCInteractiveDesc()
{
  cid_interactive_desc = microECSComponentRegister(sizeof(CInteractiveDesc),
                                                   NULL);
}

void CmpAddInteractiveDesc(int entity_id, const char *action_name,
                           const char *description, const uint32_t cost)
{
  assert(cid_interactive_desc != -1);
  microECSEntityAddComponent(entity_id, cid_interactive_desc,
                             &(CInteractiveDesc){
                               .action_name = action_name,
                               .description = description,
                               .cost = cost,
                             });
}

CInteractiveDesc *CmpGetInteractiveDesc(int entity_id)
{
  return (CInteractiveDesc *)microECSEntityGetComponent(entity_id,
                                                        cid_interactive_desc);
}

void RegisterLogicComponents()
{
  RegisterCUpdate();
  RegisterCScriptedUpdate();
  RegisterCEventListener();
  RegisterCLifetime();
  RegisterCEntityCategory();
  RegisterCName();
  RegisterCHealth();
  RegisterCInteractiveDesc();
}
