#include "AnimationSystem.h"
#include "../components/RenderingComponents.h"
#include "../ECS.h"
#include "../Graphics.h"
#include <stdlib.h>

void animationSystem(float dt) {

  CAnimation* components_animation = (CAnimation*)microECSComponentsGet(cid_animation);
  const unsigned int components_count = microECSComponentsCount(cid_animation);

  for (int i = 0; i < components_count; i++) {
    const int entityId = microECSComponentGetEntityId(cid_animation, i);
    CAnimation* animation = &components_animation[i];
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
