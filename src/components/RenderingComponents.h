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

// Text
typedef struct
{
  uint8_t fontId;
  float lineSpacing;
  char *text;
} CText;

extern int cid_text;
extern void RegisterCText();

// Color
typedef struct
{
  float r, g, b, a;
} CColor;

extern int cid_color;
extern void RegisterCColor();

// Layer
typedef struct
{
  uint8_t layerId;
  uint8_t visible;
} CDrawable;

extern int cid_drawable;
extern void RegisterCDrawable();

// Hud
typedef struct
{
} CHud;

extern int cid_hud;
extern void RegisterCHud();

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

// LockOnView
typedef struct
{
  unsigned char followRotation;
} CLockOnView;
extern int cid_lock_on_view;
extern void RegisterCLockOnView();

// ParticleEmitter
typedef struct
{
  uint16_t emitterId;
  uint16_t offsetX, offsetY;
} CParticleEmitter;
extern int cid_particle_emitter;
extern void RegisterCParticleEmitter();

extern void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
