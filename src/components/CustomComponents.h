#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <stdint.h>

typedef struct
{
  float planet_x, planet_y;
} CPlanetaryAlignment;

int cid_planetary_alignment;
void RegisterCPlanetaryAlignment();
void CmpAddPlanetaryAlignment(int entity_id, float planet_x,
                                     float planet_y);
CPlanetaryAlignment *CmpGetPlanetaryAlignment(int entity_id);

typedef struct
{
  float x, y;
} CGravity;

int cid_gravity;
void RegisterCGravity();
void CmpAddGravity(int entity_id, float x, float y);
CGravity *CmpGetGravity(int entity_id);

typedef struct
{
  float health;
  float maxHealth;
} CHealth;

int cid_health;
void RegisterCHealth();
void CmpAddHealth(int entity_id, float health, float maxHealth);
CHealth *CmpGetHealth(int entity_id);

typedef struct
{
  int damage;
} CProjectile;

int cid_projectile;
void RegisterCProjectile();
void CmpAddProjectile(int entity_id, int damage);
CProjectile *CmpGetProjectile(int entity_id);

typedef struct
{
  float interact_range;
  char *interact_text;
  void (*on_interact)(uint32_t entity_id);
} CInteractive;

int cid_interactive;
void RegisterCInteractive();
void CmpAddInteractive(int entity_id, float interact_range,
                              char *interact_text,
                              void (*on_interact)(uint32_t entity_id));
CInteractive *CmpGetInteractive(int entity_id);

typedef struct CItem
{
  uint32_t type;
  uint32_t quantity;
} CItem;

int cid_item;
void RegisterCItem();
void CmpAddItem(int entity_id, uint32_t type, uint32_t quantity);
CItem *CmpGetItem(int entity_id);

void RegisterCustomComponents();

#endif /* end of include guard: CUSTOM_COMPONENTS_H */
