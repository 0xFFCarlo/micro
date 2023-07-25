#include "CustomComponents.h"
#include "../micro/ECS.h"

int cid_planetary_alignment = -1;
int cid_gravity = -1;
int cid_health = -1;

void RegisterCPlanetaryAlignment()
{
  cid_planetary_alignment = microECSComponentRegister(
    sizeof(CPlanetaryAlignment));
}

void RegisterCGravity()
{
  cid_gravity = microECSComponentRegister(sizeof(CGravity));
}

void RegisterCHealth()
{
  cid_health = microECSComponentRegister(sizeof(CHealth));
}

void RegisterCustomComponents()
{
  RegisterCPlanetaryAlignment();
  RegisterCGravity();
  RegisterCHealth();
}
