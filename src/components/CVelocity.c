#include "CVelocity.h"
#include <stdlib.h>
#include "../ECS.h"

int cid_velocity;

void RegisterCVelocity()
{
  cid_velocity = microECSComponentRegister(sizeof(CVelocity), NULL);
}
