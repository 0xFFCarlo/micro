#include "CustomComponents.h"
#include "../ECS.h"

int cid_planetary_alignment = -1;

void RegisterCPlanetaryAlignment()
{
  cid_planetary_alignment = microECSComponentRegister(sizeof(CPlanetaryAlignment));
}

void RegisterCustomComponents()
{
  RegisterCPlanetaryAlignment();
}
