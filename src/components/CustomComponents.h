#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

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

extern void RegisterCustomComponents();

#endif /* end of include guard: CUSTOM_COMPONENTS_H */
