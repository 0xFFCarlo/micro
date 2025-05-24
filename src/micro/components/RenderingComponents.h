#ifndef RENDERING_COMPONENTS_H
#define RENDERING_COMPONENTS_H

#include "../core/Graphics.h"
#include "../util/Types.h"

// Sprite
typedef struct
{
  uint16_t textureId;
  int16_t tx, ty, tw, th;
} CSprite;

extern int cid_sprite;
void RegisterCSprite();
void CmpAddSprite(int entity_id, int textureId, int tx, int ty, int tw,
                  int th);
CSprite *CmpGetSprite(int entity_id);

// Sprite buffer
typedef struct
{
  u32 VAOId;
} CMesh;
extern int cid_mesh;
void RegisterCMesh();
void CmpAddMesh(int entity_id, int shaderId, int textureId, int vertexCount,
                int instanceCount, const MicroAttributeData *attributes,
                int attributesCount);
CMesh *CmpGetMesh(int entity_id);

// Text
typedef struct
{
  u8 fontId;
  float scale;
  float lineSpacing;
  u32 alignment;
  u32 maxLineWidth;
  char *text;
} CText;

extern int cid_text;
void RegisterCText();
void CmpAddText(int entity_id, u8 fontId, float scale, float lineSpacing,
                u32 alignment, u32 maxLineWidth, char *text);
CText *CmpGetText(int entity_id);

// Color
typedef struct
{
  unsigned char r, g, b, a;
} CColor;

extern int cid_color;
void RegisterCColor();
void CmpAddColor(int entity_id, unsigned char r, unsigned char g,
                 unsigned char b, unsigned char a);
CColor *CmpGetColor(int entity_id);

// Layer
typedef struct
{
  u8 layerId;
  bool visible;
} CDrawable;

extern int cid_drawable;
void RegisterCDrawable();
void CmpAddDrawable(int entity_id, u8 layerId, bool visible);
CDrawable *CmpGetDrawable(int entity_id);

// Hud
typedef struct
{
} CHud;

extern int cid_hud;
void RegisterCHud();
void CmpAddHud(int entity_id);
CHud *CmpGetHud(int entity_id);

// Animation
typedef struct
{
  int animationId;
  u16 frameId;
  float duration;
  bool flipX;
  bool flipY;
  bool reverse;
  float animationTime;
} CAnimation;
extern int cid_animation;
void RegisterCAnimation();
void CmpAddAnimation(int entity_id, int animationId, float duration, bool flipX,
                     bool flipY, bool reverse);
CAnimation *CmpGetAnimation(int entity_id);

// ShadedCanvas
typedef struct
{
  int width, height;
  int shaderId;
  int canvasId;
  unsigned char needsUpdate;
} CShadedCanvas;

extern int cid_shadedCanvas;
void RegisterCShadedCanvas();
void CmpAddShaderCanvas(int entity_id, int width, int height, int shaderId,
                        int canvasId);
CShadedCanvas *CmpGetShadedCanvas(int entity_id);

// LockOnView
typedef struct
{
  bool followRotation;
  bool hasBoundaries;
  float minX, minY, maxX, maxY;
} CLockOnView;
extern int cid_lock_on_view;
void RegisterCLockOnView();
void CmpAddLockOnView(int entity_id, bool followRotation, bool hasBoundaries,
                      float minX, float minY, float maxX, float maxY);
CLockOnView *CmpGetLockOnView(int entity_id);

// ParticleEmitter
typedef struct
{
  u16 emitterId;
  u16 offsetX, offsetY;
} CParticleEmitter;
extern int cid_particle_emitter;
void RegisterCParticleEmitter();
void CmpAddParticleEmitter(int entity_id, u16 emitterId, u16 offsetX,
                           u16 offsetY);
CParticleEmitter *CmpGetParticleEmitter(int entity_id);

// Light source
typedef struct
{
  int lightId;
} CLightSource;
extern int cid_light_source;
void RegisterCLightSource();
void CmpAddLightSource(int entity_id, float intensity, float radius);
CLightSource *CmpGetLightSource(int entity_id);

void RegisterRenderingComponents();

#endif /* end of include guard: RENDERING_COMPONENTS_H */
