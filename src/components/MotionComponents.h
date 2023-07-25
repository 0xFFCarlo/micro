#ifndef MOTION_COMPONENTS_H
#define MOTION_COMPONENTS_H

// Position
typedef struct
{
  double x, y;
} CPosition;

extern int cid_position;
extern void RegisterCPosition();

// Transform
typedef struct
{
  float width, height;
  float originX, originY;
  float rotation;
} CTransform;

extern int cid_transform;
extern void RegisterCTransform();

// Body
typedef struct
{
  int body_id;
} CBody;

extern int cid_body;
extern void RegisterCBody();

extern void RegisterMotionComponents();

#endif /* end of include guard: MOTION_COMPONENTS_H */
