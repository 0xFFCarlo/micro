#include "MotionComponents.h"
#include "../micro/ECS.h"
#include <stdlib.h>

int cid_position = -1;
int cid_transform = -1;
int cid_body = -1;
int cid_follow = -1;

void RegisterCPosition()
{
  cid_position = microECSComponentRegister(sizeof(CPosition));
}

void CmpAddPosition(int entity_id, double x, double y)
{
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
  cid_transform = microECSComponentRegister(sizeof(CTransform));
}

void CmpAddTransform(int entity_id, float width, float height, float originX,
                     float originY, float rotation)
{
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

void RegisterCBody()
{
  cid_body = microECSComponentRegister(sizeof(CBody));
}

void CmpAddBody(int entity_id, int body_id)
{
  microECSEntityAddComponent(entity_id, cid_body,
                             &(CBody){
                               .body_id = body_id,
                             });
}

CBody *CmpGetBody(int entity_id)
{
  return (CBody *)microECSEntityGetComponent(entity_id, cid_body);
}

void RegisterCFollow()
{
  cid_follow = microECSComponentRegister(sizeof(CFollow));
}

void CmpAddFollow(int entity_id, uint32_t target_entity_id, uint8_t lock_rot,
                  uint32_t offset_x, uint32_t offset_y)
{
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
  RegisterCFollow();
}
