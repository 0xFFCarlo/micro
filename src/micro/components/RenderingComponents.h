#ifndef RENDERING_COMPONENTS_H
#define RENDERING_COMPONENTS_H

#include "../util/Types.h"

// Sprite
typedef struct
{
  u8 textureId;
  f32 tx, ty, tw, th;
} CSprite;

int cid_sprite;
void RegisterCSprite();
void CmpAddSprite(int entity_id, u8 textureId, float tx, float ty,
                         float tw, float th);
CSprite *CmpGetSprite(int entity_id);

// Text
typedef struct
{
  u8 fontId;
  f32 lineSpacing;
  u32 alignment;
  char *text;
} CText;

int cid_text;
void RegisterCText();
void CmpAddText(int entity_id, u8 fontId, f32 lineSpacing, u32 alignment, char *text);
CText *CmpGetText(int entity_id);

// Color
typedef struct
{
  f32 r, g, b, a;
} CColor;

int cid_color;
void RegisterCColor();
void CmpAddColor(int entity_id, float r, float g, float b, float a);
CColor *CmpGetColor(int entity_id);

// Layer
typedef struct
{
  u8 layerId;
  bool visible;
} CDrawable;

int cid_drawable;
void RegisterCDrawable();
void CmpAddDrawable(int entity_id, u8 layerId, bool visible);
CDrawable *CmpGetDrawable(int entity_id);

// Hud
typedef struct
{
} CHud;

int cid_hud;
void RegisterCHud();
void CmpAddHud(int entity_id);
CHud *CmpGetHud(int entity_id);

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
int cid_animation;
void RegisterCAnimation();
void CmpAddAnimation(int entity_id, int animationId,
                            float duration, bool flipX, bool flipY,
                            bool reverse);
CAnimation *CmpGetAnimation(int entity_id);

// ShadedCanvas
typedef struct
{
  int width, height;
  int shaderId;
  int canvasId;
  unsigned char needsUpdate;
} CShadedCanvas;

int cid_shadedCanvas;
void RegisterCShadedCanvas();
void CmpAddShaderCanvas(int entity_id, int width, int height,
                               int shaderId, int canvasId);
CShadedCanvas *CmpGetShadedCanvas(int entity_id);

// LockOnView
typedef struct
{
  bool followRotation;
} CLockOnView;
int cid_lock_on_view;
void RegisterCLockOnView();
void CmpAddLockOnView(int entity_id, bool followRotation);
CLockOnView *CmpGetLockOnView(int entity_id);

// ParticleEmitter
typedef struct
{
  u16 emitterId;
  u16 offsetX, offsetY;
} CParticleEmitter;
int cid_particle_emitter;
void RegisterCParticleEmitter();
void CmpAddParticleEmitter(int entity_id, u16 emitterId, u16 offsetX,
                                  u16 offsetY);
CParticleEmitter *CmpGetParticleEmitter(int entity_id);

void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
