#include <stdlib.h>
#include "LogicComponents.h"
#include "../micro/ECS.h"

int cid_update = -1;
int cid_event_listener = -1;

void RegisterCUpdate() {
  cid_update = microECSComponentRegister(sizeof(CUpdate));
}

void RegisterCEventListener() {
  cid_event_listener = microECSComponentRegister(sizeof(CEventListener));
}

void RegisterLogicComponents() {
  RegisterCUpdate();
  RegisterCEventListener();
}
