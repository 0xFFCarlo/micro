#include "../components/MotionComponents.h"
#include "../core/ECS.h"
#include <assert.h>
#include <stdlib.h>

static void update_parent(_CParent *parent)
{
  CPosition *pos = CmpGetPosition(parent->parent_eid);
  _CParent *p = (_CParent *)CmpGetParent(parent->parent_eid);
  if (p == NULL || p->parent_eid == -1)
  {
    parent->cumulative_x = pos->x;
    parent->cumulative_y = pos->y;
  }
  else
  {
    if (!p->updated)
      update_parent(p);
    parent->cumulative_x = p->cumulative_x + pos->x;
    parent->cumulative_y = p->cumulative_y + pos->y;
  }
  parent->updated = true;
}

static void parent_system_update(float dt)
{
  (void)dt; // Unused parameter
  _CParent *components_update = (_CParent *)microECSComponentsGet(cid_parent);
  const unsigned int components_count = microECSComponentsCount(cid_parent);

  // Clean up the updated flag for all components
  for (unsigned int i = 0; i < components_count; i++)
    components_update[i].updated = false;

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_parent, i);
    if (microECSEntityIsAlive(entityId) == 0)
      continue;
    _CParent *parent = &components_update[i];
    if (!parent->updated && parent->parent_eid != -1)
      update_parent(parent);
  }
}

MicroECSSystem parent_system = {parent_system_update, NULL, NULL, NULL};
