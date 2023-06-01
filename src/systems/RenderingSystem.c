#include "RenderingSystem.h"
#include "../Graphics.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../ECS.h"
#include <stdio.h>

int sprite_system_query = -1;

int sort_drawables(int a, int b) {
  CDrawable* spriteA = (CDrawable*)microECSEntityGetComponent(a, cid_drawable);
  CDrawable* spriteB = (CDrawable*)microECSEntityGetComponent(b, cid_drawable);
  return spriteA->layerId - spriteB->layerId;
}

void renderingSystem(float dt) {
  
  if (sprite_system_query == -1) {
    int components[2] = {cid_position, cid_drawable};
    sprite_system_query = microECSCachedQueryCreate(components, 2, sort_drawables);
  }
  
  ecs_entity_list entities = microECSCachedQueryRun(sprite_system_query);
  if (entities.size == 0) return;
  
  int winWidth, winHeight;
  microWindowGetSize(&winWidth, &winHeight);

  //Store old view and set default shader
  MicroView old_view = microViewGet();
  microShaderApply(0);
  int hud_mode = 0;

  for (int i = 0; i < entities.size; i++) {

    const int entityId = entities.entityIds[i];
    CPosition* p = (CPosition*)microECSEntityGetComponent(entityId, cid_position);
    
    // Determine if entity is HUD
    const unsigned int entity_hud = microECSEntityHasComponent(entityId, cid_hud);

    // Set view either HUD or normal 
    if (entity_hud == 1 && hud_mode == 0) {
      microGraphicsDisplay();
      microViewSet((MicroView){
          .viewportX = 0,
          .viewportY = 0,
          .viewportWidth = winWidth,
          .viewportHeight = winHeight,
          .centerX = winWidth/2.0,
          .centerY = winHeight/2.0,
          .width = winWidth,
          .height = winHeight,
          .rotation = 0,
          .flipY = 0
          });
      microViewApply();
      hud_mode = 1;
    }
    else if (entity_hud == 0 && hud_mode == 1) {
      microGraphicsDisplay();
      microViewSet(old_view);
      microViewApply();
      hud_mode = 0;
    }
    
    // Draw sprite
    if (microECSEntityHasComponent(entityId, cid_sprite)) {
      CSprite* sprite = (CSprite*)microECSEntityGetComponent(entityId, cid_sprite);
      CTransform* t  = (CTransform*)microECSEntityGetComponent(entityId, cid_transform);

      // Get color
      CColor* color;
      if (microECSEntityHasComponent(entityId, cid_color))
        color  = (CColor*)microECSEntityGetComponent(entityId, cid_color);
      else
        color = &(CColor){.r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0};
      
      microGraphicsDrawRectRot(sprite->textureId,
          sprite->tx, sprite->ty, sprite->tw, sprite->th,
          p->x, p->y, t->width, t->height, t->originX,
          t->originY, t->rotation, color->r, color->g, color->b,
          color->a);
    }

    // Draw text
    if (microECSEntityHasComponent(entityId, cid_text)) {
      CText* text = (CText*)microECSEntityGetComponent(entityId, cid_text);
      
      // Get color
      CColor* color;
      if (microECSEntityHasComponent(entityId, cid_color))
        color  = (CColor*)microECSEntityGetComponent(entityId, cid_color);
      else
        color = &(CColor){.r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0};

      microGraphicsDrawText(text->fontId, text->text,
          p->x, p->y, text->lineSpacing, color->r, color->g, color->b, color->a);
    }
  } 

  microGraphicsDisplay();
  microViewSet(old_view);
  microViewApply();
}
