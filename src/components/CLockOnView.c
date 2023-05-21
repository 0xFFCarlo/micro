#include "CLockOnView.h"
#include "../ECS.h"
#include <stdio.h>

int cid_lock_on_view = -1;

void RegisterCLockOnView()
{
  cid_lock_on_view = microECSComponentRegister(sizeof(CLockOnView), NULL);
}
