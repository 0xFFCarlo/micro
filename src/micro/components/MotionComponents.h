#ifndef MOTION_COMPONENTS_H
#define MOTION_COMPONENTS_H

#include "../util/Types.h"
#include "../util/vector.h"

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

// Parent
typedef struct
{
  int parent_eid;
} CParent;

// Internal Parent
typedef struct
{
  int parent_eid;

  bool updated;
  int entity_id;
  double cumulative_x, cumulative_y;
} _CParent;

extern int cid_parent;
void RegisterCParent();
void CmpAddParent(int entity_id, int parent_eid);
CParent *CmpGetParent(int entity_id);

// Internal Childrens
typedef struct _CChildrens
{
  int *childrens;
} _CChildrens;
extern int cid_childrens;
void RegisterCChildrens();
_CChildrens *CmpGetChildrens(int entity_id);

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

void RegisterMotionComponents();

#endif /* end of include guard: MOTION_COMPONENTS_H */
