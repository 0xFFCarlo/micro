#ifndef PROJECTILE_H
#define PROJECTILE_H

typedef enum ProjectileType {
  PROJECTILE_TYPE_PLAYER,
  PROJECTILE_TYPE_ENEMY,
} ProjectileType;

extern void ProjectileAddEntity(ProjectileType type, const int x, const int y, const int vx,
                                const int vy);

#endif /* end of include guard: PROJECTILE_H */
