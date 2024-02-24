#include "CustomComponents.h"
#include "../micro/core/ECS.h"

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

void CmpAddPlanetaryAlignment(int entity_id, float planet_x, float planet_y)
{
  microECSEntityAddComponent(entity_id, cid_planetary_alignment,
                             &(CPlanetaryAlignment){
                               .planet_x = planet_x,
                               .planet_y = planet_y,
                             });
}

CPlanetaryAlignment *CmpGetPlanetaryAlignment(int entity_id)
{
  return (CPlanetaryAlignment *)
    microECSEntityGetComponent(entity_id, cid_planetary_alignment);
}

void RegisterCGravity()
{
  cid_gravity = microECSComponentRegister(sizeof(CGravity));
}

void CmpAddGravity(int entity_id, float x, float y)
{
  microECSEntityAddComponent(entity_id, cid_gravity,
                             &(CGravity){
                               .x = x,
                               .y = y,
                             });
}

CGravity *CmpGetGravity(int entity_id)
{
  return (CGravity *)microECSEntityGetComponent(entity_id, cid_gravity);
}

void RegisterCHealth()
{
  cid_health = microECSComponentRegister(sizeof(CHealth));
}

void CmpAddHealth(int entity_id, float health, float maxHealth)
{
  microECSEntityAddComponent(entity_id, cid_health,
                             &(CHealth){
                               .health = health,
                               .maxHealth = maxHealth,
                             });
}

CHealth *CmpGetHealth(int entity_id)
{
  return (CHealth *)microECSEntityGetComponent(entity_id, cid_health);
}

void RegisterCProjectile()
{
  cid_projectile = microECSComponentRegister(sizeof(CProjectile));
}

void CmpAddProjectile(int entity_id, int damage)
{
  microECSEntityAddComponent(entity_id, cid_projectile,
                             &(CProjectile){
                               .damage = damage,
                             });
}

CProjectile *CmpGetProjectile(int entity_id)
{
  return (CProjectile *)microECSEntityGetComponent(entity_id, cid_projectile);
}

void RegisterCInteractive()
{
  cid_interactive = microECSComponentRegister(sizeof(CInteractive));
}

void CmpAddInteractive(int entity_id, float interact_range, char *interact_text,
                       void (*on_interact)(uint32_t entity_id))
{
  microECSEntityAddComponent(entity_id, cid_interactive,
                             &(CInteractive){
                               .interact_range = interact_range,
                               .interact_text = interact_text,
                               .on_interact = on_interact,
                             });
}

CInteractive *CmpGetInteractive(int entity_id)
{
  return (CInteractive *)microECSEntityGetComponent(entity_id, cid_interactive);
}

void RegisterCItem()
{
  cid_item = microECSComponentRegister(sizeof(CItem));
}

void CmpAddItem(int entity_id, uint32_t type, uint32_t quantity)
{
  microECSEntityAddComponent(entity_id, cid_item,
                             &(CItem){
                               .type = type,
                               .quantity = quantity,
                             });
}

CItem *CmpGetItem(int entity_id)
{
  return (CItem *)microECSEntityGetComponent(entity_id, cid_item);
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
