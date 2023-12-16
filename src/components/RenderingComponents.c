#include "RenderingComponents.h"
#include "../micro/ECS.h"
#include <stdlib.h>

int cid_sprite = -1;
int cid_text = -1;
int cid_color = -1;
int cid_drawable = -1;
int cid_hud = -1;
int cid_animation = -1;
int cid_shadedCanvas = -1;
int cid_lock_on_view = -1;
int cid_particle_emitter = -1;

void RegisterCSprite()
{
  cid_sprite = microECSComponentRegister(sizeof(CSprite));
}

void CmpAddSprite(int entity_id, u8 textureId, float tx, float ty,
                  float tw, float th)
{
  microECSEntityAddComponent(entity_id, cid_sprite,
                             &(CSprite){
                               .textureId = textureId,
                               .tx = tx,
                               .ty = ty,
                               .tw = tw,
                               .th = th,
                             });
}

CSprite *CmpGetSprite(int entity_id)
{
  return (CSprite *)microECSEntityGetComponent(entity_id, cid_sprite);
}

void RegisterCText()
{
  cid_text = microECSComponentRegister(sizeof(CText));
}

void CmpAddText(int entity_id, u8 fontId, f32 lineSpacing, u32 alignment, char *text)
{
  microECSEntityAddComponent(entity_id, cid_text,
                             &(CText){
                               .fontId = fontId,
                               .lineSpacing = lineSpacing,
                               .alignment = alignment,
                               .text = text,
                             });
}

CText *CmpGetText(int entity_id)
{
  return (CText *)microECSEntityGetComponent(entity_id, cid_text);
}

void RegisterCColor()
{
  cid_color = microECSComponentRegister(sizeof(CColor));
}

void CmpAddColor(int entity_id, float r, float g, float b, float a)
{
  microECSEntityAddComponent(entity_id, cid_color,
                             &(CColor){
                               .r = r,
                               .g = g,
                               .b = b,
                               .a = a,
                             });
}

CColor *CmpGetColor(int entity_id)
{
  return (CColor *)microECSEntityGetComponent(entity_id, cid_color);
}

void RegisterCDrawable()
{
  cid_drawable = microECSComponentRegister(sizeof(CDrawable));
}

void CmpAddDrawable(int entity_id, u8 layerId, bool visible)
{
  microECSEntityAddComponent(entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = layerId,
                               .visible = visible,
                             });
}

CDrawable *CmpGetDrawable(int entity_id)
{
  return (CDrawable *)microECSEntityGetComponent(entity_id, cid_drawable);
}

void RegisterCHud()
{
  cid_hud = microECSComponentRegister(sizeof(CHud));
}

void CmpAddHud(int entity_id)
{
  microECSEntityAddComponent(entity_id, cid_hud, &(CHud){});
}

CHud *CmpGetHud(int entity_id)
{
  return (CHud *)microECSEntityGetComponent(entity_id, cid_hud);
}

void RegisterCAnimation()
{
  cid_animation = microECSComponentRegister(sizeof(CAnimation));
}

void CmpAddAnimation(int entity_id, int animationId,
                            float duration, bool flipX, bool flipY,
                            bool reverse)
{
  microECSEntityAddComponent(entity_id, cid_animation,
                             &(CAnimation){
                               .animationId = animationId,
                               .duration = duration,
                               .flipX = flipX,
                               .flipY = flipY,
                               .reverse = reverse,
                               .animationTime = 0,
                               .frameId = 0,
                             });
}

CAnimation *CmpGetAnimation(int entity_id)
{
  return (CAnimation *)microECSEntityGetComponent(entity_id, cid_animation);
}

void RegisterCShadedCanvas()
{
  cid_shadedCanvas = microECSComponentRegister(sizeof(CShadedCanvas));
}

void CmpAddShaderCanvas(int entity_id, int width, int height, int shaderId,
                        int canvasId)
{
  microECSEntityAddComponent(entity_id, cid_shadedCanvas,
                             &(CShadedCanvas){
                               .width = width,
                               .height = height,
                               .shaderId = shaderId,
                               .canvasId = canvasId,
                               .needsUpdate = 1,
                             });
}

CShadedCanvas *CmpGetShadedCanvas(int entity_id)
{
  return (CShadedCanvas *)microECSEntityGetComponent(entity_id,
                                                     cid_shadedCanvas);
}

void RegisterCLockOnView()
{
  cid_lock_on_view = microECSComponentRegister(sizeof(CLockOnView));
}

void CmpAddLockOnView(int entity_id, bool followRotation)
{
  microECSEntityAddComponent(entity_id, cid_lock_on_view,
                             &(CLockOnView){
                               .followRotation = followRotation,
                             });
}

CLockOnView *CmpGetLockOnView(int entity_id)
{
  return (CLockOnView *)microECSEntityGetComponent(entity_id, cid_lock_on_view);
}

void RegisterCParticleEmitter()
{
  cid_particle_emitter = microECSComponentRegister(sizeof(CParticleEmitter));
}

void CmpAddParticleEmitter(int entity_id, uint16_t emitterId, uint16_t offsetX,
                           uint16_t offsetY)
{
  microECSEntityAddComponent(entity_id, cid_particle_emitter,
                             &(CParticleEmitter){
                               .emitterId = emitterId,
                               .offsetX = offsetX,
                               .offsetY = offsetY,
                             });
}

CParticleEmitter *CmpGetParticleEmitter(int entity_id)
{
  return (CParticleEmitter *)microECSEntityGetComponent(entity_id,
                                                        cid_particle_emitter);
}

void RegisterRenderingComponents()
{
  RegisterCSprite();
  RegisterCText();
  RegisterCColor();
  RegisterCDrawable();
  RegisterCHud();
  RegisterCAnimation();
  RegisterCShadedCanvas();
  RegisterCLockOnView();
  RegisterCParticleEmitter();
}
