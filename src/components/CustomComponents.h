#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <stdint.h>

typedef struct
{
  float planet_x, planet_y;
} CPlanetaryAlignment;

extern int cid_planetary_alignment;
extern void RegisterCPlanetaryAlignment();
extern void CmpAddPlanetaryAlignment(int entity_id, float planet_x,
                                     float planet_y);
extern CPlanetaryAlignment *CmpGetPlanetaryAlignment(int entity_id);

typedef struct
{
  float x, y;
} CGravity;

extern int cid_gravity;
extern void RegisterCGravity();
extern void CmpAddGravity(int entity_id, float x, float y);
extern CGravity *CmpGetGravity(int entity_id);

typedef struct
{
  float health;
  float maxHealth;
} CHealth;

extern int cid_health;
extern void RegisterCHealth();
extern void CmpAddHealth(int entity_id, float health, float maxHealth);
extern CHealth *CmpGetHealth(int entity_id);

typedef struct
{
  int damage;
} CProjectile;

extern int cid_projectile;
extern void RegisterCProjectile();
extern void CmpAddProjectile(int entity_id, int damage);
extern CProjectile *CmpGetProjectile(int entity_id);

typedef struct
{
  float interact_range;
  char *interact_text;
  void (*on_interact)(uint32_t entity_id);
} CInteractive;

extern int cid_interactive;
extern void RegisterCInteractive();
extern void CmpAddInteractive(int entity_id, float interact_range,
                              char *interact_text,
                              void (*on_interact)(uint32_t entity_id));
extern CInteractive *CmpGetInteractive(int entity_id);

typedef struct CItem
{
  uint32_t type;
  uint32_t quantity;
} CItem;

extern int cid_item;
extern void RegisterCItem();
extern void CmpAddItem(int entity_id, uint32_t type, uint32_t quantity);
extern CItem *CmpGetItem(int entity_id);

extern void RegisterCustomComponents();

#endif /* end of include guard: CUSTOM_COMPONENTS_H */
