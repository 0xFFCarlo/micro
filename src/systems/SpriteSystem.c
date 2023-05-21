#include "SpriteSystem.h"
#include "../Graphics.h"
#include "../components/CPosition.h"
#include "../components/CSprite.h"
#include "../ECS.h"

int sprite_system_query = -1;

int sort_sprites(int a, int b) {
  CSprite* spriteA = (CSprite*)microECSEntityGetComponent(a, cid_sprite);
  CSprite* spriteB = (CSprite*)microECSEntityGetComponent(b, cid_sprite);
  return spriteA->layerId - spriteB->layerId;
}

void spriteSystem(float dt) {

  if (sprite_system_query == -1) {
    int components[2] = {cid_position, cid_sprite};
    sprite_system_query = microECSQueryCreate(components, 2, sort_sprites);
  }
  
  ecs_entity_list entities = microECSQueryRun(sprite_system_query);
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
    CSprite* sprite = (CSprite*)microECSEntityGetComponent(entityId, cid_sprite);

  
    if (sprite->hud == 1 && hud_mode == 0) {
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
    else if (sprite->hud == 0 && hud_mode == 1) {
      microGraphicsDisplay();
      microViewSet(old_view);
      microViewApply();
      hud_mode = 0;
    }

    microGraphicsDrawRectRot(sprite->textureId,
        sprite->tx, sprite->ty, sprite->tw, sprite->th,
        p->x, p->y, sprite->width, sprite->height, sprite->originX,
        sprite->originY, sprite->rotation, sprite->r, sprite->g, sprite->b,
        sprite->a);
  } 

  microGraphicsDisplay();
  microViewSet(old_view);
  microViewApply();
}
