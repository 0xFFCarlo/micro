#include "MotionComponents.h"
#include "../core/ECS.h"
#include "../core/Physics.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define WORLD_ID_MAIN 0
#define WORLD_ID_INTERACTIVE 1

int cid_position = -1;
int cid_transform = -1;
int cid_parent = -1;
int cid_childrens = -1;
int cid_body = -1;
int cid_follow = -1;
int cid_interactable = -1;

void RegisterCPosition()
{
  cid_position = microECSComponentRegister(sizeof(CPosition), NULL);
}

void CmpAddPosition(int entity_id, double x, double y)
{
  assert(cid_position != -1);
  microECSEntityAddComponent(entity_id, cid_position,
                             &(CPosition){
                               .x = x,
                               .y = y,
                             });
}

CPosition *CmpGetPosition(int entity_id)
{
  return (CPosition *)microECSEntityGetComponent(entity_id, cid_position);
}

void RegisterCTransform()
{
  cid_transform = microECSComponentRegister(sizeof(CTransform), NULL);
}

void CmpAddTransform(int entity_id, int width, int height, float originX,
                     float originY, float rotation)
{
  assert(cid_transform != -1);
  assert(width >= 0);
  assert(height >= 0);
  microECSEntityAddComponent(entity_id, cid_transform,
                             &(CTransform){
                               .width = width,
                               .height = height,
                               .originX = originX,
                               .originY = originY,
                               .rotation = rotation,
                             });
}

CTransform *CmpGetTransform(int entity_id)
{
  return (CTransform *)microECSEntityGetComponent(entity_id, cid_transform);
}

void FreeCChildrens(void *childrens)
{
  _CChildrens *cchildrens = (_CChildrens *)childrens;
  // Remove all children entities
  while (vec_len(cchildrens->childrens) > 0)
  {
    int child_id = *(int *)vec_back(cchildrens->childrens);
    vec_pop_back(cchildrens->childrens);
    CParent *parent_cmp = CmpGetParent(child_id);
    parent_cmp->parent_eid = -1; // Clear parent component
    microECSEntityQueueFree(child_id);
  }

  // Free vector
  vec_free(cchildrens->childrens);
}

void RegisterCChildrens()
{
  cid_childrens = microECSComponentRegister(sizeof(_CChildrens),
                                            FreeCChildrens);
}

_CChildrens *CmpGetChildrens(int entity_id)
{
  _CChildrens *childrens = (_CChildrens *)
    microECSEntityGetComponent(entity_id, cid_childrens);
  return childrens;
}

void CmpAddChildrens(int entity_id)
{
  assert(cid_childrens != -1);
  _CChildrens *childrens = CmpGetChildrens(entity_id);
  assert(childrens == NULL);
  microECSEntityAddComponent(entity_id, cid_childrens,
                             &(_CChildrens){
                               .childrens = vec_new(sizeof(int)),
                             });
}

void FreeCParent(void *parent)
{
  _CParent *cparent = (_CParent *)parent;
  if (cparent->parent_eid == -1)
    return; // No parent to free
  _CChildrens *childrens = CmpGetChildrens(cparent->parent_eid);
  assert(childrens != NULL);
  for (size_t i = 0; i < vec_len(childrens->childrens); i++)
  {
    const int child_id = childrens->childrens[i];
    if (child_id != cparent->entity_id)
      continue;
    vec_remove_at(childrens->childrens, i);
    break; // Found and removed the child
  }
}

void RegisterCParent()
{
  cid_parent = microECSComponentRegister(sizeof(_CParent), FreeCParent);
}

void CmpAddParent(int entity_id, int parent_eid)
{
  assert(entity_id != -1);
  assert(parent_eid != -1);
  CParent *parent_cmp = CmpGetParent(entity_id);
  assert(parent_cmp == NULL);
  microECSEntityAddComponent(entity_id, cid_parent,
                             &(_CParent){
                               .parent_eid = parent_eid,
                               .entity_id = entity_id,
                             });
  _CChildrens *childrens = CmpGetChildrens(parent_eid);
  if (childrens == NULL)
  {
    CmpAddChildrens(parent_eid);
    childrens = CmpGetChildrens(parent_eid);
  }
  vec_append(childrens->childrens, &entity_id);
}

CParent *CmpGetParent(int entity_id)
{
  return (CParent *)microECSEntityGetComponent(entity_id, cid_parent);
}

void FreeCBody(void *body)
{
  CBody *cbody = (CBody *)body;
  microPhysicsBodyFree(cbody->body_id);
}

void RegisterCBody()
{
  cid_body = microECSComponentRegister(sizeof(CBody), FreeCBody);
}

void CmpAddBodyCircle(int entity_id, float cx, float cy, float radius,
                      float mass, int isStatic, uint8_t canRotate,
                      float elasticity, float friction)
{
  assert(cid_body != -1);
  const int body_id = microPhysicsBodyNewCircle(entity_id, WORLD_ID_MAIN, cx,
                                                cy, radius, mass, isStatic,
                                                canRotate, elasticity,
                                                friction);
  microECSEntityAddComponent(entity_id, cid_body,
                             &(CBody){
                               .body_id = body_id,
                             });
}

void CmpAddBodyRect(int entity_id, float cx, float cy, float width,
                    float height, float mass, int isStatic, uint8_t canRotate,
                    float elasticity, float friction)
{
  assert(cid_body != -1);
  const int body_id = microPhysicsBodyNewRect(entity_id, WORLD_ID_MAIN, cx, cy,
                                              width, height, mass, isStatic,
                                              canRotate, elasticity, friction);
  microECSEntityAddComponent(entity_id, cid_body,
                             &(CBody){
                               .body_id = body_id,
                             });
}

CBody *CmpGetBody(int entity_id)
{
  return (CBody *)microECSEntityGetComponent(entity_id, cid_body);
}

void FreeCInteractable(void *interactable)
{
  CInteractable *cinteractable = (CInteractable *)interactable;
  microPhysicsBodyFree(cinteractable->_sensor_body_id);
}

void RegisterCInteractable()
{
  cid_interactable = microECSComponentRegister(sizeof(CInteractable),
                                               FreeCInteractable);
}

void CmpAddInteractable(int entity_id, bool isActor, float range, float offsetX,
                        float offsetY, bool isStatic,
                        bool (*interact)(int, int),
                        void (*on_in_range)(int, int))
{
  assert(cid_interactable != -1);
  CPosition *pos = CmpGetPosition(entity_id);
  assert(pos != NULL);
  const int sensor_body_id = microPhysicsBodyNewCircle(entity_id,
                                                       WORLD_ID_INTERACTIVE,
                                                       pos->x + offsetX,
                                                       pos->y + offsetY, range,
                                                       1.0, isStatic, false, 0,
                                                       0);
  microPhysicsBodySetSensor(sensor_body_id, true);

  if (on_in_range)
    microPhysicsBodySetCollisionUpdateCallback(sensor_body_id, on_in_range);

  // Actor collides with all categories execept itself (0)
  // Non-actor collides only with actor (1) category
  if (isActor)
    microPhysicsBodySetFilter(sensor_body_id, 1, 0b10);
  else
    microPhysicsBodySetFilter(sensor_body_id, 2, 0b01);

  microECSEntityAddComponent(entity_id, cid_interactable,
                             &(CInteractable){
                               .interact = interact,
                               ._offsetX = offsetX,
                               ._offsetY = offsetY,
                               ._sensor_body_id = sensor_body_id,
                             });
}

void CmpAddInteractableRect(int entity_id, bool isActor, float width,
                            float height, float offsetX, float offsetY,
                            bool isStatic, bool (*interact)(int, int),
                            void (*on_in_range)(int, int))
{
  assert(cid_interactable != -1);
  CPosition *pos = CmpGetPosition(entity_id);
  assert(pos != NULL);
  const int sensor_body_id = microPhysicsBodyNewRect(entity_id,
                                                     WORLD_ID_INTERACTIVE,
                                                     pos->x + offsetX,
                                                     pos->y + offsetY, width,
                                                     height, 1.0, isStatic,
                                                     false, 0, 0);
  microPhysicsBodySetSensor(sensor_body_id, true);
  if (on_in_range)
    microPhysicsBodySetCollisionUpdateCallback(sensor_body_id, on_in_range);

  // Actor collides with all categories execept itself (0)
  // Non-actor collides only with actor (1) category
  if (isActor)
    microPhysicsBodySetFilter(sensor_body_id, 1, 0b10);
  else
    microPhysicsBodySetFilter(sensor_body_id, 2, 0b01);

  microECSEntityAddComponent(entity_id, cid_interactable,
                             &(CInteractable){
                               .interact = interact,
                               ._offsetX = offsetX,
                               ._offsetY = offsetY,
                               ._sensor_body_id = sensor_body_id,
                             });
}

CInteractable *CmpGetInteractable(int entity_id)
{
  return (CInteractable *)microECSEntityGetComponent(entity_id,
                                                     cid_interactable);
}

void RegisterMotionComponents()
{
  RegisterCPosition();
  RegisterCTransform();
  RegisterCParent();
  RegisterCChildrens();
  RegisterCBody();
  RegisterCInteractable();
}
