#include "MotionComponents.h"
#include "../micro/ECS.h"
#include <stdlib.h>

int cid_position = -1;
int cid_transform = -1;
int cid_body = -1;

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

void RegisterMotionComponents()
{
  RegisterCPosition();
  RegisterCTransform();
  RegisterCBody();
}
