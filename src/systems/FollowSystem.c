#include "FollowSystem.h"
#include "../components/MotionComponents.h"
#include "../micro/ECS.h"
#include "../util/debug.h"

void followSystem(float dt)
{
  (void)dt;

  CFollow *components = (CFollow *)microECSComponentsGet(cid_follow);
  const unsigned int components_count = microECSComponentsCount(cid_follow);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_follow, i);
    CPosition *position = CmpGetPosition(entityId);
    assert(position != NULL);
    CPosition *follow_position = CmpGetPosition(components[i].target_entity_id);
    assert(follow_position != NULL);
    position->x = follow_position->x + components[i].offset_x;
    position->y = follow_position->y + components[i].offset_y;

    if (components[i].lock_rot)
    {
      CTransform *transform = CmpGetTransform(entityId);
      assert(transform != NULL);
      CTransform *follow_transform = CmpGetTransform(components[i]
                                                       .target_entity_id);
      assert(follow_transform != NULL);
      transform->rotation = follow_transform->rotation;
    }
  }
}
