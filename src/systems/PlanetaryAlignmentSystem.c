#include "PlanetaryAlignmentSystem.h"
#include "../components/CustomComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include <math.h>

void planetaryAligntmentSystem(float dt)
{
  (void)(dt); // Unused parameter

  CPlanetaryAlignment *components_planetary_alignment = (CPlanetaryAlignment *)
    microECSComponentsGet(cid_planetary_alignment);
  const u32 components_count = microECSComponentsCount(cid_planetary_alignment);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_planetary_alignment,
                                                      i);
    CPosition *position = CmpGetPosition(entityId);
    CTransform *transform = CmpGetTransform(entityId);

    // Update rotation based on planet position
    float outVecX = position->x - components_planetary_alignment[i].planet_x;
    float outVecY = position->y - components_planetary_alignment[i].planet_y;
    float outVecLen = sqrt(outVecX * outVecX + outVecY * outVecY);
    outVecX /= outVecLen;
    outVecY /= outVecLen;
    float rotation_rad = atan2(outVecY, outVecX) + M_PI / 2.0;
    transform->rotation = rotation_rad;

    // Update body rotation if it exists
    if (microECSEntityHasComponent(entityId, cid_body))
    {
      CBody *body = CmpGetBody(entityId);
      float curr_rotation = microPhysicsBodyGetRotation(body->body_id);
      if (fabs(curr_rotation - rotation_rad) > 0.001)
        microPhysicsBodySetRotation(body->body_id, rotation_rad);
    }
  }
}
