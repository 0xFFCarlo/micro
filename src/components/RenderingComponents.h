#ifndef RENDERING_COMPONENTS_H
#define RENDERING_COMPONENTS_H

#include "../micro/Types.h"

// Sprite
typedef struct
{
  u8 textureId;
  f32 tx, ty, tw, th;
} CSprite;

extern int cid_sprite;
extern void RegisterCSprite();
extern void CmpAddSprite(int entity_id, u8 textureId, float tx, float ty,
                         float tw, float th);
extern CSprite *CmpGetSprite(int entity_id);

// Text
typedef struct
{
  u8 fontId;
  f32 lineSpacing;
  char *text;
} CText;

extern int cid_text;
extern void RegisterCText();
extern void CmpAddText(int entity_id, u8 fontId, f32 lineSpacing, char *text);
extern CText *CmpGetText(int entity_id);

// Color
typedef struct
{
  f32 r, g, b, a;
} CColor;

extern int cid_color;
extern void RegisterCColor();
extern void CmpAddColor(int entity_id, float r, float g, float b, float a);
extern CColor *CmpGetColor(int entity_id);

// Layer
typedef struct
{
  u8 layerId;
  bool visible;
} CDrawable;

extern int cid_drawable;
extern void RegisterCDrawable();
extern void CmpAddDrawable(int entity_id, u8 layerId, bool visible);
extern CDrawable *CmpGetDrawable(int entity_id);

// Hud
typedef struct
{
} CHud;

extern int cid_hud;
extern void RegisterCHud();
extern void CmpAddHud(int entity_id);
extern CHud *CmpGetHud(int entity_id);

// Animation
typedef struct
{
  int animationId;
  u16 frameId;
  f32 duration;
  bool flipX;
  bool flipY;
  bool reverse;
  float animationTime;
} CAnimation;
extern int cid_animation;
extern void RegisterCAnimation();
extern void CmpAddAnimation(int entity_id, int animationId,
                            float duration, bool flipX, bool flipY,
                            bool reverse);
extern CAnimation *CmpGetAnimation(int entity_id);

// ShadedCanvas
typedef struct
{
  int width, height;
  int shaderId;
  int canvasId;
  unsigned char needsUpdate;
} CShadedCanvas;

extern int cid_shadedCanvas;
extern void RegisterCShadedCanvas();
extern void CmpAddShaderCanvas(int entity_id, int width, int height,
                               int shaderId, int canvasId);
extern CShadedCanvas *CmpGetShadedCanvas(int entity_id);

// LockOnView
typedef struct
{
  bool followRotation;
} CLockOnView;
extern int cid_lock_on_view;
extern void RegisterCLockOnView();
extern void CmpAddLockOnView(int entity_id, bool followRotation);
extern CLockOnView *CmpGetLockOnView(int entity_id);

// ParticleEmitter
typedef struct
{
  u16 emitterId;
  u16 offsetX, offsetY;
} CParticleEmitter;
extern int cid_particle_emitter;
extern void RegisterCParticleEmitter();
extern void CmpAddParticleEmitter(int entity_id, u16 emitterId, u16 offsetX,
                                  u16 offsetY);
extern CParticleEmitter *CmpGetParticleEmitter(int entity_id);

extern void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
