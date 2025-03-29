#include "UI.h"
#include "../components/Components.h"
#include "../core/ECS.h"
#include "../util/debug.h"
#include <string.h>

#define MICRO_UI_MAX_CONTAINERS 256
#define MICRO_UI_MAX_CHILDREN 16

typedef struct MicroUIContainer
{
  MicroUIContainerDesc desc;
  bool needsUpdate;
  bool isSelected;
  int type_eid;

  struct MicroUIContainer *parent;
  struct MicroUIContainer *child[MICRO_UI_MAX_CHILDREN];
  int depth;
  int child_count;
} MicroUIContainer;

static MicroUIContainer containers[MICRO_UI_MAX_CONTAINERS];
static int containersCount = 0;

int microUIContainerNew(MicroUIContainerDesc desc)
{
  int parent_id = -1;
  for (int i = 0; i < containersCount; i++)
  {
    if (containers[i].desc.width != 0)
      continue;
    parent_id = i;
    break;
  }

  if (parent_id == -1)
  {
    parent_id = containersCount;
    containersCount++;
  }
  assert(parent_id < MICRO_UI_MAX_CONTAINERS);

  MicroUIContainer *container = &containers[parent_id];
  container->desc = desc;
  container->needsUpdate = true;
  container->parent = NULL;
  container->type_eid = -1;
  container->child_count = 0;
  container->depth = 0;
  return parent_id;
}

void microUIContaineChildNew(int parentId, int childId)
{
  MicroUIContainer *parent = &containers[parentId];
  MicroUIContainer *child = &containers[childId];
  containers[childId].depth = parent->depth + 1;
  parent->child[parent->child_count] = child;
  parent->child_count++;
  child->parent = parent;
}

void microUIContainerChildRemove(int parentId, int childId)
{
  MicroUIContainer *parent = &containers[parentId];
  MicroUIContainer *child = &containers[childId];
  for (int i = 0; i < parent->child_count; i++)
  {
    if (parent->child[i] != child)
      continue;

    parent->child[i] = parent->child[parent->child_count - 1];
    parent->child_count--;
    child->parent = NULL;
    child->depth = 0;
    return;
  }
}

int MicroUIContainerGetChildCount(int containerId)
{
  return containers[containerId].child_count;
}

int microUIContainerGetChild(int containerId, int childIndex)
{
  return (containers[containerId].child[childIndex] - containers);
}

void microUIContainerSet(int containerId, MicroUIContainerDesc desc)
{
  containers[containerId].desc = desc;
  containers[containerId].needsUpdate = true;
}

MicroUIContainerDesc microUIContainerGet(int containerId)
{
  return containers[containerId].desc;
}

MicroUIContainerDesc *microUIContainerModify(int containerId)
{
  containers[containerId].needsUpdate = true;
  return &containers[containerId].desc;
}

void microUIContainerFree(int containerId)
{
  for (int i = 0; i < containers[containerId].child_count; i++)
    microUIContainerFree(containers[containerId].child[i] - containers);
  containers[containerId].desc.width = 0;
  containers[containerId].child_count = 0;
  containers[containerId].needsUpdate = false;
  containers[containerId].isSelected = false;
  containers[containerId].parent = NULL;
  if (containers[containerId].type_eid != -1)
  {
    microECSEntityRemove(containers[containerId].type_eid);
    containers[containerId].type_eid = -1;
  }
}

static void microUIUpdateContainer(MicroUIContainer *container)
{
  if (container->needsUpdate == false)
    return;

  container->needsUpdate = false;
  MicroUIContainerDesc *desc = &container->desc;

  // Make sure container entity exists
  if (container->type_eid == -1 && desc->type != MICRO_UI_CONT_TYPE_NONE)
  {
    if (desc->type != MICRO_UI_CONT_TYPE_NONE)
    {
      container->type_eid = microECSEntityNew(NULL, NULL);
      CmpAddPosition(container->type_eid, 0, 0);
      CmpAddDrawable(container->type_eid, 8 + container->depth, true);
      CmpAddTransform(container->type_eid, desc->width, desc->height, 0, 0, 0);
      CmpAddHud(container->type_eid);
    }
    if (desc->type == MICRO_UI_CONT_TYPE_SPRITE)
    {
      CmpAddSprite(container->type_eid, desc->sprite.textureId,
                   desc->sprite.tsrc.x, desc->sprite.tsrc.y,
                   desc->sprite.tsrc.w, desc->sprite.tsrc.h);
      CmpAddColor(container->type_eid, desc->sprite.color[0],
                  desc->sprite.color[1], desc->sprite.color[2],
                  desc->sprite.color[3]);
    }
    else if (desc->type == MICRO_UI_CONT_TYPE_TEXT)
    {
      CmpAddText(container->type_eid, desc->text.fontId, desc->text.fontSize,
                 1.0, desc->text.alignment, desc->width, desc->text.text);
      CmpAddColor(container->type_eid, desc->text.color[0], desc->text.color[1],
                  desc->text.color[2], desc->text.color[3]);
      if (desc->text.alignment == TEXT_ALIGN_CENTER)
      {
        CTransform *transform = CmpGetTransform(container->type_eid);
        transform->originX = desc->width / 2.f;
      }
      else if (desc->text.alignment == TEXT_ALIGN_RIGHT)
      {
        CTransform *transform = CmpGetTransform(container->type_eid);
        transform->originX = desc->width;
      }
    }
  }

  if (desc->type != MICRO_UI_CONT_TYPE_NONE)
  {
    // Update entity
    // TODO: only update when needed
    CPosition *pos = CmpGetPosition(container->type_eid);
    pos->x = desc->x;
    pos->y = desc->y;
    CTransform *transform = CmpGetTransform(container->type_eid);
    transform->width = desc->width;
    transform->height = desc->width;
  }

  // Update childrens positions
  for (int i = 0; i < container->child_count; i++)
  {
    MicroUIContainer *child = container->child[i];
    if (i == 0)
    {
      child->desc.x = desc->x + desc->margin[0];
      child->desc.y = desc->y + desc->margin[1];
    }
    else
    {
      MicroUIContainer *prev = container->child[i - 1];
      if (desc->layout == MICRO_UI_LAYOUT_HORIZONTAL)
      {
        child->desc.x = prev->desc.x + prev->desc.width + prev->desc.padding[2];
        child->desc.y = desc->y + desc->margin[1];
      }
      else
      {
        child->desc.y = prev->desc.y + prev->desc.height +
                        prev->desc.padding[3];
        child->desc.x = desc->x + desc->margin[0];
      }
    }
    microUIUpdateContainer(child);
  }
}

void microUIUpdate()
{
  for (int i = 0; i < containersCount; i++)
  {
    MicroUIContainer *container = &containers[i];

    // Search for root nodes
    if (container->parent != NULL)
      continue;

    microUIUpdateContainer(container);
  }
}
