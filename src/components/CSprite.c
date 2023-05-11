#include "CSprite.h"
#include "../ECS.h"

int cid_sprite;

void RegisterCSprite()
{
  cid_sprite = microECSComponentRegister(sizeof(CSprite));
}
