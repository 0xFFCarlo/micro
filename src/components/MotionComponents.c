#include <stdlib.h>
#include "MotionComponents.h"
#include "../ECS.h"

int cid_position = -1;
int cid_transform = -1;
int cid_body = -1;

void RegisterCPosition()
{
  cid_position = microECSComponentRegister(sizeof(CPosition));
}

void RegisterCTransform()
{
  cid_transform = microECSComponentRegister(sizeof(CTransform));
}

void RegisterCBody()
{
  cid_body = microECSComponentRegister(sizeof(CBody));
}

void RegisterMotionComponents()
{
  RegisterCPosition();
  RegisterCTransform();
  RegisterCBody();
}
