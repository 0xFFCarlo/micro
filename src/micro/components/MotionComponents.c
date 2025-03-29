#include "MotionComponents.h"
#include "../core/ECS.h"
#include "../core/Physics.h"
#include <assert.h>
#include <stdlib.h>

#define WORLD_ID_MAIN 0
#define WORLD_ID_INTERACTIVE 1

int cid_position = -1;
int cid_transform = -1;
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

void RegisterCFollow()
{
  cid_follow = microECSComponentRegister(sizeof(CFollow), NULL);
}

void CmpAddFollow(int entity_id, uint32_t target_entity_id, uint8_t lock_rot,
                  int32_t offset_x, int32_t offset_y)
{
  assert(cid_follow != -1);
  microECSEntityAddComponent(entity_id, cid_follow,
                             &(CFollow){
                               .target_entity_id = target_entity_id,
                               .lock_rot = lock_rot,
                               .offset_x = offset_x,
                               .offset_y = offset_y,
                             });
}

CFollow *CmpGetFollow(int entity_id)
{
  return (CFollow *)microECSEntityGetComponent(entity_id, cid_follow);
}

void RegisterMotionComponents()
{
  RegisterCPosition();
  RegisterCTransform();
  RegisterCBody();
  RegisterCInteractable();
  RegisterCFollow();
}
