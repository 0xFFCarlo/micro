#include "LogicComponents.h"
#include "../micro/ECS.h"
#include <stdlib.h>

int cid_update = -1;
int cid_event_listener = -1;
int cid_lifetime = -1;

void RegisterCUpdate()
{
  cid_update = microECSComponentRegister(sizeof(CUpdate));
}

void RegisterCEventListener()
{
  cid_event_listener = microECSComponentRegister(sizeof(CEventListener));
}

void RegisterCLifetime()
{
  cid_lifetime = microECSComponentRegister(sizeof(CLifetime));
}

void RegisterLogicComponents()
{
  RegisterCUpdate();
  RegisterCEventListener();
  RegisterCLifetime();
}
