#include "CustomComponents.h"
#include "../micro/ECS.h"

int cid_planetary_alignment = -1;
int cid_gravity = -1;
int cid_health = -1;
int cid_projectile = -1;
int cid_interactive = -1;
int cid_item = -1;

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

void RegisterCProjectile()
{
  cid_projectile = microECSComponentRegister(sizeof(CProjectile));
}

void RegisterCInteractive()
{
  cid_interactive = microECSComponentRegister(sizeof(CInteractive));
}

void RegisterCItem()
{
  cid_item = microECSComponentRegister(sizeof(CItem));
}

void RegisterCustomComponents()
{
  RegisterCPlanetaryAlignment();
  RegisterCGravity();
  RegisterCHealth();
  RegisterCProjectile();
  RegisterCInteractive();
  RegisterCItem();
}
