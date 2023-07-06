#include "AnimationSystem.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include <stdlib.h>

void animationSystem(float dt) {

  CAnimation* components_animation = (CAnimation*)microECSComponentsGet(cid_animation);
  const unsigned int components_count = microECSComponentsCount(cid_animation);

  for (int i = 0; i < components_count; i++) {
    const int entityId = microECSComponentGetEntityId(cid_animation, i);
    CAnimation* animation = &components_animation[i];
    CSprite* sprite = (CSprite*)microECSEntityGetComponent(entityId, cid_sprite);
    int framesCount = microAnimationGetFramesCount(animation->animationId);
    float frameDuration = animation->framesDuration / (float)framesCount;

    // Update animation frame
    animation->timeSinceLastFrame += dt;
    if (animation->timeSinceLastFrame > frameDuration) {
      animation->timeSinceLastFrame = 0;
      animation->frameId = (animation->frameId + 1) % framesCount;
    }

    // Update texture source
    MicroTextureSource source = microAnimationGetFrame(animation->animationId, animation->frameId, animation->flipX, animation->flipY);
    sprite->tx = source.x;
    sprite->ty = source.y;
    sprite->tw = source.w;
    sprite->th = source.h;
  }
}
