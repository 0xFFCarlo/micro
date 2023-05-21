#include <stdlib.h>
#include "CAnimation.h"
#include "../ECS.h"

int cid_animation = -1;

void RegisterCAnimation() {
  cid_animation = microECSComponentRegister(sizeof(CAnimation), NULL);
}
