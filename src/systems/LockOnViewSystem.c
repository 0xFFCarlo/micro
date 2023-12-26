#include "LockOnViewSystem.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include <stdio.h>
#include <stdlib.h>

void lockOnViewSystem(float dt)
{
  (void)(dt); // Unused parameter

  CLockOnView *components_lock_on_view = (CLockOnView *)
    microECSComponentsGet(cid_lock_on_view);
  const unsigned int
    components_count = microECSComponentsCount(cid_lock_on_view);

  if (components_count > 1)
    printf("Warning: more than one entity with lock on view component\n");

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_lock_on_view, i);
    CLockOnView *lockOnView = &components_lock_on_view[i];
    CPosition *position = (CPosition *)microECSEntityGetComponent(entityId,
                                                                  cid_position);

    microViewSetCenter(position->x, position->y);

    if (lockOnView->followRotation > 0)
    {
      CTransform *t = (CTransform *)microECSEntityGetComponent(entityId,
                                                               cid_transform);
      microViewSetRotation(t->rotation);
    }
  }

  microViewApply();
}
