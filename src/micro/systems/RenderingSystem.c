#include "RenderingSystem.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include "../core/System.h"
#include "../util/debug.h"
#include <stdio.h>

#define MICRO_MAX_LAYER 32

static int hud_mode = 0;

typedef struct
{
  int *entities;
  bool is_sorted;
  int *culled_entities;
  int *y_bins[16];
} draw_layer_t;

static draw_layer_t draw_layers[MICRO_MAX_LAYER] = {0};

static inline int compare_drawables(int a, int b)
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

static void insertion_sort(int arr[], int n)
{
  for (int i = 1; i < n; i++)
  {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && compare_drawables(arr[j], key) > 0)
    {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

static void rendering_system_quit()
{
  for (int lid = 0; lid < MICRO_MAX_LAYER; lid++)
  {
    draw_layer_t *layer = &draw_layers[lid];
    if (layer->entities != NULL)
      vec_free(layer->entities);
  }
}

static void rendering_system_update(float dt)
{
  (void)(dt); // Unused parameter

  // TODO:
  // - clear bins without deallocating
  // - bin all drawables by layerId
  // - go through each bin and check which one require Y-sorting
  // - cull drawables, have a bin for drawables not in viewport which
  //   do not require Y-sorting, OR Y-bin based on y relative to viewport.y
  //   if in viewport
  // - sort each Y-bin by Y using custom insertion sort (no function callback)
  // - move sorted entities in a single array? Might be nice, just a couple of
  // memcpy

  int winWidth, winHeight;
  microSystemGetWindowSize(&winWidth, &winHeight);

  // Store old view and set default shader
  const MicroView old_view = *microViewGet();
  hud_mode = 0;

  // Clear layers
  for (int lid = 0; lid < MICRO_MAX_LAYER; lid++)
  {
    draw_layer_t *layer = &draw_layers[lid];
    if (layer->entities == NULL)
      layer->entities = vec_new(sizeof(int));
    else
      vec_empty(layer->entities);
    layer->is_sorted = false;
  }

  // Get all entities with drawable component
  CDrawable *drawable = microECSComponentsGet(cid_drawable);
  size_t drawable_count = microECSComponentsCount(cid_drawable);
  for (size_t i = 0; i < drawable_count; i++)
  {
    int entityId = microECSComponentGetEntityId(cid_drawable, i);
    if (!microECSEntityIsAlive(entityId))
      continue;
    if (drawable[i].layerId < 0 || drawable[i].layerId >= MICRO_MAX_LAYER)
    {
      debug_print("Entity %d has invalid layerId %d\n", entityId,
                  drawable[i].layerId);
      continue;
    }
    vec_append(draw_layers[drawable[i].layerId].entities, &entityId);
  }

  // Sort entities in each layer
  for (int lid = 0; lid < MICRO_MAX_LAYER; lid++)
  {
    draw_layer_t *layer = &draw_layers[lid];
    if (vec_len(layer->entities) == 0)
      continue;
    // Sort entities by Y position
    insertion_sort(layer->entities, vec_len(layer->entities));
    layer->is_sorted = true;
  }

  for (int lid = 0; lid < MICRO_MAX_LAYER; lid++)
  {
    draw_layer_t *layer = &draw_layers[lid];

    for (int i = 0; i < (int)vec_len(layer->entities); i++)
    {
      const int entityId = layer->entities[i];

      // Check if the entity was removed during the game loop
      if (microECSEntityIsAlive(entityId) == false)
        continue;

      const CDrawable *drawable = CmpGetDrawable(entityId);

      // Should the entity be drawn?
      if (drawable->visible == false)
        continue;

      const CPosition *p = CmpGetPosition(entityId);
      double posX = p->x;
      double posY = p->y;

      if (microECSEntityHasComponent(entityId, cid_parent))
      {
        _CParent *parent = (_CParent *)CmpGetParent(entityId);
        posX += parent->cumulative_x;
        posY += parent->cumulative_y;
      }

      // Determine if entity is HUD
      const unsigned int entity_hud = microECSEntityHasComponent(entityId,
                                                                 cid_hud);

      // Set view either HUD or normal
      if (entity_hud == 1 && hud_mode == 0)
      {
        microGraphicsDisplay();
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
        microViewApply(microGraphicsGetSpriteShaderId());
        hud_mode = 1;
      }
      else if (entity_hud == 0 && hud_mode == 1)
      {
        microGraphicsDisplay();
        microViewSet(old_view);
        microViewApply(microGraphicsGetSpriteShaderId());
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
                                sprite->tw, sprite->th, posX, posY, t->width,
                                t->height, t->originX, t->originY, t->rotation,
                                color->r, color->g, color->b, color->a);
      }

      // Draw sprite buffer
      if (microECSEntityHasComponent(entityId, cid_mesh))
      {
        // Draw pending sprites first
        microGraphicsDisplay();

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

        microGraphicsDrawText(text->fontId, text->text, posX + originX,
                              posY + originY, text->lineSpacing, text->scale,
                              text->alignment, text->maxLineWidth, color->r,
                              color->g, color->b, color->a);
      }
    }
  }

  microGraphicsDisplay();
}

MicroECSSystem rendering_system = {rendering_system_update,
                                   rendering_system_quit, NULL, NULL};
