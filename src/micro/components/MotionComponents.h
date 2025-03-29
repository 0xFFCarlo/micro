#ifndef MOTION_COMPONENTS_H
#define MOTION_COMPONENTS_H

#include "../util/Types.h"

// Position
typedef struct
{
  double x, y;
} CPosition;

extern int cid_position;
void RegisterCPosition();
void CmpAddPosition(int entity_id, double x, double y);
CPosition *CmpGetPosition(int entity_id);

// Transform
typedef struct
{
  int width, height;
  float originX, originY;
  float rotation;
} CTransform;

extern int cid_transform;
void RegisterCTransform();
void CmpAddTransform(int entity_id, int width, int height, float originX,
                     float originY, float rotation);
CTransform *CmpGetTransform(int entity_id);

// Body
typedef struct
{
  int body_id;
} CBody;

extern int cid_body;
void RegisterCBody();
void CmpAddBodyCircle(int entity_id, float cx, float cy, float radius,
                      float mass, int isStatic, uint8_t canRotate,
                      float elasticity, float friction);
void CmpAddBodyRect(int entity_id, float cx, float cy, float width,
                    float height, float mass, int isStatic, uint8_t canRotate,
                    float elasticity, float friction);
CBody *CmpGetBody(int entity_id);

// Interactable, detect if an entity is in range to interact with another entity
typedef struct
{
  bool (*interact)(int, int);
  float _offsetX, _offsetY;
  int _sensor_body_id;
} CInteractable;
extern int cid_interactable;
void RegisterCInteractable();
void CmpAddInteractable(int entity_id, bool isActor, float range, float offsetX,
                        float offsetY, bool isStatic,
                        bool (*interact)(int, int),
                        void (*on_in_range)(int, int));
void CmpAddInteractableRect(int entity_id, bool isActor, float width,
                            float height, float offsetX, float offsetY,
                            bool isStatic, bool (*interact)(int, int),
                            void (*on_in_range)(int, int));
CInteractable *CmpGetInteractable(int entity_id);

// Follow an entity
typedef struct
{
  uint32_t target_entity_id;
  uint8_t lock_rot;
  int32_t offset_x;
  int32_t offset_y;
} CFollow;

extern int cid_follow;
void RegisterCFollow();
void CmpAddFollow(int entity_id, uint32_t target_entity_id, uint8_t lock_rot,
                  int32_t offset_x, int32_t offset_y);
CFollow *CmpGetFollow(int entity_id);

void RegisterMotionComponents();

#endif /* end of include guard: MOTION_COMPONENTS_H */
