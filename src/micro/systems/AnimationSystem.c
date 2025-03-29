#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include <math.h>
#include <stdlib.h>

static void animation_system_update(float dt)
{

  CAnimation *components_animation = (CAnimation *)
    microECSComponentsGet(cid_animation);
  const unsigned int components_count = microECSComponentsCount(cid_animation);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_animation, i);
    CAnimation *animation = &components_animation[i];
    if (animation->animationId < 0)
      continue;
    CSprite *sprite = (CSprite *)microECSEntityGetComponent(entityId,
                                                            cid_sprite);
    int framesCount = microAnimationGetFramesCount(animation->animationId);
    float frameDuration = animation->duration / (float)framesCount;

    // Update animation time
    if (animation->reverse)
      animation->animationTime -= dt;
    else
      animation->animationTime += dt;

    if (animation->animationTime > animation->duration)
      animation->animationTime -= animation->duration;
    else if (animation->animationTime < 0)
      animation->animationTime += animation->duration;

    // Update animation frame
    animation->frameId = (int)floorf(animation->animationTime / frameDuration);

    // Update texture source
    MicroTextureRegion source = microAnimationGetFrame(animation->animationId,
                                                       animation->frameId,
                                                       animation->flipX,
                                                       animation->flipY);
    sprite->tx = source.x;
    sprite->ty = source.y;
    sprite->tw = source.w;
    sprite->th = source.h;
  }
}

MicroECSSystem animation_system = {animation_system_update, NULL, NULL};
