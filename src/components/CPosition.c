#include <stdlib.h>
#include "CPosition.h"
#include "../ECS.h"

int cid_position;

void RegisterCPosition()
{
  cid_position = microECSComponentRegister(sizeof(CPosition), NULL);
}


