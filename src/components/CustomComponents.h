#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <stdint.h>

typedef struct
{
  float planet_x, planet_y;
} CPlanetaryAlignment;

extern int cid_planetary_alignment;
extern void RegisterCPlanetaryAlignment();

typedef struct
{
  float x, y;
} CGravity;

extern int cid_gravity;
extern void RegisterCGravity();

typedef struct
{
  int health;
  int maxHealth;
} CHealth;

extern int cid_health;
extern void RegisterCHealth();

typedef struct
{
  int damage;
} CProjectile;

extern int cid_projectile;
extern void RegisterCProjectile();

typedef struct
{
  uint8_t can_interact;
  float interact_range;
  void (*on_interact)(uint32_t entity_id);
} CInteractive;

extern int cid_interactive;
extern void RegisterCInteractive();

typedef struct CItem
{
  uint32_t type;
  uint32_t quantity;
} CItem;

extern int cid_item;
extern void RegisterCItem();

extern void RegisterCustomComponents();

#endif /* end of include guard: CUSTOM_COMPONENTS_H */
