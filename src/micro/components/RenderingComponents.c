#include "RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include "MotionComponents.h"
#include <assert.h>
#include <stdlib.h>

int cid_sprite = -1;
int cid_mesh = -1;
int cid_text = -1;
int cid_color = -1;
int cid_drawable = -1;
int cid_hud = -1;
int cid_animation = -1;
int cid_shadedCanvas = -1;
int cid_lock_on_view = -1;
int cid_particle_emitter = -1;
int cid_light_source = -1;

void RegisterCSprite()
{
  cid_sprite = microECSComponentRegister(sizeof(CSprite), NULL);
}

void CmpAddSprite(int entity_id, u16 textureId, float tx, float ty, float tw,
                  float th)
{
  assert(cid_sprite != -1);
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

void FreeCMesh(void *component)
{
  CMesh *mesh = (CMesh *)component;
  microVAOFree(mesh->VAOId);
}

void RegisterCMesh()
{
  cid_mesh = microECSComponentRegister(sizeof(CMesh), FreeCMesh);
}

void CmpAddMesh(int entity_id, int shaderId, int textureId, int vertexCount,
                int instanceCount, const MicroAttributeData *attributes,
                int attributesCount)
{
  assert(cid_mesh != -1);
  u32 vao_id = microVAONew(shaderId, textureId, vertexCount, instanceCount,
                           attributes, attributesCount);
  microECSEntityAddComponent(entity_id, cid_mesh,
                             &(CMesh){
                               .VAOId = vao_id,
                             });
}

CMesh *CmpGetMesh(int entity_id)
{
  return (CMesh *)microECSEntityGetComponent(entity_id, cid_mesh);
}

void RegisterCText()
{
  cid_text = microECSComponentRegister(sizeof(CText), NULL);
}

void CmpAddText(int entity_id, u8 fontId, float scale, float lineSpacing,
                u32 alignment, u32 maxLineWidth, char *text)
{
  assert(cid_text != -1);
  microECSEntityAddComponent(entity_id, cid_text,
                             &(CText){
                               .fontId = fontId,
                               .scale = scale,
                               .lineSpacing = lineSpacing,
                               .alignment = alignment,
                               .maxLineWidth = maxLineWidth,
                               .text = text,
                             });
}

CText *CmpGetText(int entity_id)
{
  return (CText *)microECSEntityGetComponent(entity_id, cid_text);
}

void RegisterCColor()
{
  cid_color = microECSComponentRegister(sizeof(CColor), NULL);
}

void CmpAddColor(int entity_id, unsigned char r, unsigned char g,
                 unsigned char b, unsigned char a)
{
  assert(cid_color != -1);
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
  cid_drawable = microECSComponentRegister(sizeof(CDrawable), NULL);
}

void CmpAddDrawable(int entity_id, u8 layerId, bool visible)
{
  assert(cid_drawable != -1);
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
  cid_hud = microECSComponentRegister(sizeof(CHud), NULL);
}

void CmpAddHud(int entity_id)
{
  assert(cid_hud != -1);
  microECSEntityAddComponent(entity_id, cid_hud, &(CHud){});
}

CHud *CmpGetHud(int entity_id)
{
  return (CHud *)microECSEntityGetComponent(entity_id, cid_hud);
}

void RegisterCAnimation()
{
  cid_animation = microECSComponentRegister(sizeof(CAnimation), NULL);
}

void CmpAddAnimation(int entity_id, int animationId, float duration, bool flipX,
                     bool flipY, bool reverse)
{
  assert(cid_animation != -1);
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
  cid_shadedCanvas = microECSComponentRegister(sizeof(CShadedCanvas), NULL);
}

void CmpAddShaderCanvas(int entity_id, int width, int height, int shaderId,
                        int canvasId)
{
  assert(cid_shadedCanvas != -1);
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
  cid_lock_on_view = microECSComponentRegister(sizeof(CLockOnView), NULL);
}

void CmpAddLockOnView(int entity_id, bool followRotation, bool hasBoundaries,
                      float minX, float minY, float maxX, float maxY)
{
  assert(cid_lock_on_view != -1);
  microECSEntityAddComponent(entity_id, cid_lock_on_view,
                             &(CLockOnView){
                               .followRotation = followRotation,
                               .hasBoundaries = hasBoundaries,
                               .minX = minX,
                               .minY = minY,
                               .maxX = maxX,
                               .maxY = maxY,
                             });
}

CLockOnView *CmpGetLockOnView(int entity_id)
{
  return (CLockOnView *)microECSEntityGetComponent(entity_id, cid_lock_on_view);
}

void RegisterCParticleEmitter()
{
  cid_particle_emitter = microECSComponentRegister(sizeof(CParticleEmitter),
                                                   NULL);
}

void CmpAddParticleEmitter(int entity_id, uint16_t emitterId, uint16_t offsetX,
                           uint16_t offsetY)
{
  assert(cid_particle_emitter != -1);
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

void FreeCLightSource(void *component)
{
  CLightSource *lightSource = (CLightSource *)component;
  microLightRemove(lightSource->lightId);
}

void RegisterCLightSource()
{
  cid_light_source = microECSComponentRegister(sizeof(CLightSource),
                                               FreeCLightSource);
}

void CmpAddLightSource(int entity_id, float intensity, float radius)
{
  assert(cid_light_source != -1);
  CPosition *position = CmpGetPosition(entity_id);
  assert(position != NULL);
  int lightId = microLightAdd(position->x, position->y, radius, intensity);
  microECSEntityAddComponent(entity_id, cid_light_source,
                             &(CLightSource){.lightId = lightId});
}

CLightSource *CmpGetLightSource(int entity_id)
{
  return (CLightSource *)microECSEntityGetComponent(entity_id,
                                                    cid_light_source);
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
  RegisterCLightSource();
  RegisterCMesh();
}
