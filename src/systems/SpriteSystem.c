#include "SpriteSystem.h"
#include "../Graphics.h"
#include "../components/CPosition.h"
#include "../components/CSprite.h"
#include "../ECS.h"

void spriteSystem(float a) {
  
  int components[2] = {cid_position, cid_sprite};
  ecs_component_query query = microECSQueryByComponents(components, 2);
  if (query.size == 0) return;
  
  for (int i = 0; i < query.size; i++) {
    const int entityId = query.entityIds[i];
    CPosition* p = (CPosition*)microECSEntityGetComponent(entityId, cid_position);
    CSprite* sprite = (CSprite*)microECSEntityGetComponent(entityId, cid_sprite);
    // printf("Drawing sprite %d\n", entityId);
    // printf("  position: %f, %f\n", p->x, p->y);
    // printf("  sprite: %f, %f, %f, %f\n", sprite->tx, sprite->ty, sprite->tw, sprite->th);
    // printf("  sprite: %f, %f, %f, %f\n", sprite->width, sprite->height, sprite->originX, sprite->originY);
    // printf("  sprite: %f, %f, %f, %f\n", sprite->rotation, sprite->r, sprite->g, sprite->b);
    // printf("  sprite: %f\n", sprite->a);
    
    // microGraphicsDrawRect(sprite->textureId,
    //     sprite->tx, sprite->ty, sprite->tw, sprite->th,
    //     p->x, p->y, sprite->width, sprite->height,
    //     sprite->r, sprite->g, sprite->b, sprite->a);
    
    microGraphicsDrawRectRot(sprite->textureId,
        sprite->tx, sprite->ty, sprite->tw, sprite->th,
        p->x, p->y, sprite->width, sprite->height, sprite->originX,
        sprite->originY, sprite->rotation, sprite->r, sprite->g, sprite->b,
        sprite->a);
  } 
}
