#ifndef RENDERING_COMPONENTS_H
#define RENDERING_COMPONENTS_H

#include <stdint.h>

// Sprite
typedef struct
{
  uint8_t textureId;
  float tx, ty, tw, th;
} CSprite;

extern int cid_sprite;
extern void RegisterCSprite();
extern void CmpAddSprite(int entity_id, uint8_t textureId, float tx, float ty,
                         float tw, float th);
extern CSprite *CmpGetSprite(int entity_id);

// Text
typedef struct
{
  uint8_t fontId;
  float lineSpacing;
  char *text;
} CText;

extern int cid_text;
extern void RegisterCText();
extern void CmpAddText(int entity_id, uint8_t fontId, float lineSpacing,
                       char *text);
extern CText *CmpGetText(int entity_id);

// Color
typedef struct
{
  float r, g, b, a;
} CColor;

extern int cid_color;
extern void RegisterCColor();
extern void CmpAddColor(int entity_id, float r, float g, float b, float a);
extern CColor *CmpGetColor(int entity_id);

// Layer
typedef struct
{
  uint8_t layerId;
  uint8_t visible;
} CDrawable;

extern int cid_drawable;
extern void RegisterCDrawable();
extern void CmpAddDrawable(int entity_id, uint8_t layerId, uint8_t visible);
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
  uint16_t frameId;
  float framesDuration;
  uint8_t flipX;
  uint8_t flipY;
  float timeSinceLastFrame;
} CAnimation;
extern int cid_animation;
extern void RegisterCAnimation();
extern void CmpAddAnimation(int entity_id, int animationId,
                            float framesDuration, uint8_t flipX, uint8_t flipY);
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
  uint8_t followRotation;
} CLockOnView;
extern int cid_lock_on_view;
extern void RegisterCLockOnView();
extern void CmpAddLockOnView(int entity_id, uint8_t followRotation);
extern CLockOnView *CmpGetLockOnView(int entity_id);

// ParticleEmitter
typedef struct
{
  uint16_t emitterId;
  uint16_t offsetX, offsetY;
} CParticleEmitter;
extern int cid_particle_emitter;
extern void RegisterCParticleEmitter();
extern void CmpAddParticleEmitter(int entity_id, uint16_t emitterId,
                                  uint16_t offsetX, uint16_t offsetY);
extern CParticleEmitter *CmpGetParticleEmitter(int entity_id);

extern void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
