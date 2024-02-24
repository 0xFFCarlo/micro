#ifndef MOTION_COMPONENTS_H
#define MOTION_COMPONENTS_H

#include <stdint.h>

// Position
typedef struct
{
  double x, y;
} CPosition;

int cid_position;
void RegisterCPosition();
void CmpAddPosition(int entity_id, double x, double y);
CPosition *CmpGetPosition(int entity_id);

// Transform
typedef struct
{
  float width, height;
  float originX, originY;
  float rotation;
} CTransform;

int cid_transform;
void RegisterCTransform();
void CmpAddTransform(int entity_id, float width, float height,
                            float originX, float originY, float rotation);
CTransform *CmpGetTransform(int entity_id);

// Body
typedef struct
{
  int body_id;
} CBody;

int cid_body;
void RegisterCBody();
void CmpAddBody(int entity_id, int body_id);
CBody *CmpGetBody(int entity_id);

typedef struct
{
  uint32_t target_entity_id;
  uint8_t lock_rot;
  uint32_t offset_x;
  uint32_t offset_y;
} CFollow;

int cid_follow;
void RegisterCFollow();
void CmpAddFollow(int entity_id, uint32_t target_entity_id,
                         uint8_t lock_rot, uint32_t offset_x,
                         uint32_t offset_y);
CFollow *CmpGetFollow(int entity_id);

void RegisterMotionComponents();

#endif /* end of include guard: MOTION_COMPONENTS_H */
