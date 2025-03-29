#include "RenderingSystem.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include "../core/System.h"
#include "../util/debug.h"
#include <stdio.h>

static int sprite_system_query = -1;
static int shader_id = 0;
static int hud_mode = 0;

int sort_drawables(int a, int b)
{
  const CDrawable *spriteA = CmpGetDrawable(a);
  const CDrawable *spriteB = CmpGetDrawable(b);
  const CPosition *posA = CmpGetPosition(a);
  const CPosition *posB = CmpGetPosition(b);
  int tmp = spriteA->layerId - spriteB->layerId;
  if (tmp == 0)
    return posA->y - posB->y;
  return tmp;
}

void renderingSystemSetShader(int shader)
{
  shader_id = shader;
}

void rendering_system_update(float dt)
{
  (void)(dt); // Unused parameter

  if (sprite_system_query == -1)
  {
    int components[2] = {cid_position, cid_drawable};
    sprite_system_query = microECSCachedQueryCreate(components, 2,
                                                    sort_drawables);
  }

  ecs_entity_list entities = microECSCachedQueryRun(sprite_system_query);
  if (entities.size == 0)
    return;

  int winWidth, winHeight;
  microSystemGetWindowSize(&winWidth, &winHeight);

  // Store old view and set default shader
  MicroView old_view = microViewGet();
  microShaderApply(shader_id);
  hud_mode = 0;

  for (int i = 0; i < entities.size; i++)
  {
    const int entityId = entities.entityIds[i];
    if (microECSEntityIsAlive(entityId) == 0)
      continue;
    const CPosition *p = CmpGetPosition(entityId);
    const CDrawable *drawable = CmpGetDrawable(entityId);

    // Check if the entity was removed during the game loop
    if (microECSEntityIsAlive(entityId) == false)
      continue;

    // Should the entity be drawn?
    if (drawable->visible == false)
      continue;

    // Determine if entity is HUD
    const unsigned int entity_hud = microECSEntityHasComponent(entityId,
                                                               cid_hud);

    // Set view either HUD or normal
    if (entity_hud == 1 && hud_mode == 0)
    {
      microGraphicsDisplay();
      microShaderApply(shader_id);
      microViewSet((MicroView){.viewportX = 0,
                               .viewportY = 0,
                               .viewportWidth = winWidth,
                               .viewportHeight = winHeight,
                               .centerX = winWidth / 2.0,
                               .centerY = winHeight / 2.0,
                               .width = winWidth,
                               .height = winHeight,
                               .rotation = 0,
                               .flipY = 0});
      microViewApply();
      hud_mode = 1;
    }
    else if (entity_hud == 0 && hud_mode == 1)
    {
      microGraphicsDisplay();
      microShaderApply(shader_id);
      microViewSet(old_view);
      microViewApply();
      hud_mode = 0;
    }

    // Draw sprite
    if (microECSEntityHasComponent(entityId, cid_sprite))
    {
      const CSprite *sprite = CmpGetSprite(entityId);
      const CTransform *t = CmpGetTransform(entityId);

      // Get color
      CColor *color;
      if (microECSEntityHasComponent(entityId, cid_color))
        color = CmpGetColor(entityId);
      else
        color = &(CColor){.r = 255, .g = 255, .b = 255, .a = 255};

      microGraphicsDrawSprite(sprite->textureId, sprite->tx, sprite->ty,
                              sprite->tw, sprite->th, p->x, p->y, t->width,
                              t->height, t->originX, t->originY, t->rotation,
                              color->r, color->g, color->b, color->a);
    }

    // Draw sprite buffer
    if (microECSEntityHasComponent(entityId, cid_mesh))
    {
      const CMesh *mesh = CmpGetMesh(entityId);
      microVAODraw(mesh->VAOId);
    }

    // Draw particles
    if (microECSEntityHasComponent(entityId, cid_particle_emitter))
    {
      const CParticleEmitter *emitter = CmpGetParticleEmitter(entityId);
      microParticleEmitterDraw(emitter->emitterId);
    }

    // Draw text
    if (microECSEntityHasComponent(entityId, cid_text))
    {
      const CText *text = CmpGetText(entityId);

      // Get color
      CColor *color;
      if (microECSEntityHasComponent(entityId, cid_color))
        color = CmpGetColor(entityId);
      else
        color = &(CColor){.r = 255, .g = 255, .b = 255, .a = 255};

      int originX = 0, originY = 0;
      if (microECSEntityHasComponent(entityId, cid_transform))
      {
        const CTransform *transform = CmpGetTransform(entityId);
        originX = transform->originX;
        originY = transform->originY;
      }

      microGraphicsDrawText(text->fontId, text->text, p->x + originX,
                            p->y + originY, text->lineSpacing, text->scale,
                            text->alignment, text->maxLineWidth, color->r,
                            color->g, color->b, color->a);
    }
  }

  microGraphicsDisplay();
  microViewSet(old_view);
  microViewApply();
}

MicroECSSystem rendering_system = {rendering_system_update, NULL, NULL};
