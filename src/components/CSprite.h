#ifndef CSPRITE_H
#define CSPRITE_H

typedef struct
{
  int textureId;
  float tx, ty, tw, th;
  float width, height;
  float originX, originY;
  float rotation;
  float r, g, b, a;
  int layerId;
} CSprite;

extern int cid_sprite;
extern void RegisterCSprite();

#endif /* end of include guard: CSPRITE_H */
