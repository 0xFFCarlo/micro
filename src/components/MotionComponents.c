#include <stdlib.h>
#include "MotionComponents.h"
#include "../ECS.h"

int cid_position = -1;
int cid_transform = -1;
int cid_velocity = -1;

void RegisterCPosition()
{
  cid_position = microECSComponentRegister(sizeof(CPosition), NULL);
}

void RegisterCTransform()
{
  cid_transform = microECSComponentRegister(sizeof(CTransform), NULL);
}

void RegisterCVelocity()
{
  cid_velocity = microECSComponentRegister(sizeof(CVelocity), NULL);
}
