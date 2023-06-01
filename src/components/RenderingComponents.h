#ifndef RENDERING_COMPONENTS_H
#define RENDERING_COMPONENTS_H

// Sprite
typedef struct {
    int textureId;
    float tx, ty, tw, th;
} CSprite;

extern int cid_sprite;
extern void RegisterCSprite();


// Text 
typedef struct {
  int fontId;
  float lineSpacing;
  char* text;
} CText;

extern int cid_text;
extern void RegisterCText();


// Color
typedef struct {
    float r, g, b, a;
} CColor;

extern int cid_color;
extern void RegisterCColor();


// Layer
typedef struct {
    int layerId;
} CDrawable;

extern int cid_drawable;
extern void RegisterCDrawable();


// Hud
typedef struct {} CHud;

extern int cid_hud;
extern void RegisterCHud();


// Animation
typedef struct {
    int animationId;
    int frameId;
    float timeSinceLastFrame;
} CAnimation;

extern int cid_animation;
extern void RegisterCAnimation();


// ShadedCanvas
typedef struct {
  int width, height;
  int shaderId;
  int canvasId;
} CShadedCanvas;

extern int cid_shadedCanvas;
extern void RegisterCShadedCanvas();


// LockOnView
typedef struct {
  unsigned char followRotation;
} CLockOnView;

extern int cid_lock_on_view;
extern void RegisterCLockOnView();

extern void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
