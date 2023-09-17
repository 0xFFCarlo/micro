#ifndef MOTION_COMPONENTS_H
#define MOTION_COMPONENTS_H

#include <stdint.h>

// Position
typedef struct
{
  double x, y;
} CPosition;

extern int cid_position;
extern void RegisterCPosition();
extern void CmpAddPosition(int entity_id, double x, double y);
CPosition *CmpGetPosition(int entity_id);

// Transform
typedef struct
{
  float width, height;
  float originX, originY;
  float rotation;
} CTransform;

extern int cid_transform;
extern void RegisterCTransform();
extern void CmpAddTransform(int entity_id, float width, float height,
                            float originX, float originY, float rotation);
CTransform *CmpGetTransform(int entity_id);

// Body
typedef struct
{
  int body_id;
} CBody;

extern int cid_body;
extern void RegisterCBody();
extern void CmpAddBody(int entity_id, int body_id);
CBody *CmpGetBody(int entity_id);

typedef struct
{
  uint32_t target_entity_id;
  uint8_t lock_rot;
  uint32_t offset_x;
  uint32_t offset_y;
} CFollow;

extern int cid_follow;
extern void RegisterCFollow();
extern void CmpAddFollow(int entity_id, uint32_t target_entity_id,
                         uint8_t lock_rot, uint32_t offset_x,
                         uint32_t offset_y);
CFollow *CmpGetFollow(int entity_id);

extern void RegisterMotionComponents();

#endif /* end of include guard: MOTION_COMPONENTS_H */
