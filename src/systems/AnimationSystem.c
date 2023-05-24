#include "AnimationSystem.h"
#include "../components/RenderingComponents.h"
#include "../ECS.h"
#include "../Graphics.h"
#include <stdlib.h>

int animation_system_query = -1;

void animationSystem(float dt)
{
  if (animation_system_query == -1)
  {
    int components[2] = {cid_animation, cid_sprite};
    animation_system_query = microECSQueryCreate(components, 2, NULL);
  }

  ecs_entity_list entities = microECSQueryRun(animation_system_query);
  if (entities.size == 0)
    return;

  for (int i = 0; i < entities.size; i++) {
    const int entityId = entities.entityIds[i];
    CAnimation* animation = (CAnimation*)microECSEntityGetComponent(entityId, cid_animation);
    CSprite* sprite = (CSprite*)microECSEntityGetComponent(entityId, cid_sprite);
    int framesCount = microAnimationGetFramesCount(animation->animationId);
    float frameDuration = microAnimationGetSpeed(animation->animationId) / (float)framesCount;

    // Update animation frame
    animation->timeSinceLastFrame += dt;
    if (animation->timeSinceLastFrame > frameDuration) {
      animation->timeSinceLastFrame = 0;
      animation->frameId = (animation->frameId + 1) % framesCount;
    }

    // Update texture source
    int startX, startY, width, height;
    microAnimationGetStart(animation->animationId, &startX, &startY);
    microAnimationGetFrameSize(animation->animationId, &width, &height);
    int flipX = microAnimationGetFlipX(animation->animationId);
    int flipY = microAnimationGetFlipY(animation->animationId);

    sprite->tx = startX + animation->frameId * width;
    sprite->ty = startY;
    sprite->tw = width;
    sprite->th = height;

    if (flipX) {
      sprite->tx += width;
      sprite->tw = -width;
    }

    if (flipY) {
      sprite->ty += height;
      sprite->th = -height;
    }
  }
}
