#include <stdlib.h>
#include "LogicComponents.h"
#include "../ECS.h"

int cid_update = -1;

void RegisterCUpdate() {
  cid_update = microECSComponentRegister(sizeof(CUpdate), NULL);
}
