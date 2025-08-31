#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../core/ECS.h"
#include "../core/Graphics.h"
#include <stdio.h>
#include <stdlib.h>

static void lock_on_view_system_update(float dt)
{
  (void)(dt); // Unused parameter

  CLockOnView *components_lock_on_view = (CLockOnView *)
    microECSComponentsGet(cid_lock_on_view);
  const unsigned int
    components_count = microECSComponentsCount(cid_lock_on_view);

  if (components_count > 1)
    printf("Warning: more than one entity with lock on view component\n");

  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_lock_on_view, i);
    CLockOnView *lockOnView = &components_lock_on_view[i];
    CPosition *position = (CPosition *)microECSEntityGetComponent(entityId,
                                                                  cid_position);

    float viewCX = position->x;
    float viewCY = position->y;
    if (lockOnView->hasBoundaries)
    {
      viewCX = MAX(viewCX, lockOnView->minX + viewWidth / 2);
      viewCX = MIN(viewCX, lockOnView->maxX - viewWidth / 2);
      viewCY = MAX(viewCY, lockOnView->minY + viewHeight / 2);
      viewCY = MIN(viewCY, lockOnView->maxY - viewHeight / 2);
    }
    microViewSetCenter(viewCX, viewCY);

    if (lockOnView->followRotation > 0)
    {
      CTransform *t = (CTransform *)microECSEntityGetComponent(entityId,
                                                               cid_transform);
      microViewSetRotation(t->rotation);
    }
  }

  microViewApply(microGraphicsGetSpriteShaderId());
}

MicroECSSystem lock_on_view_system = {lock_on_view_system_update, NULL, NULL, NULL};

