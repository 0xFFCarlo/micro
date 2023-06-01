#include "PlanetaryAlignmentSystem.h"
#include "../components/CustomComponents.h"
#include "../components/RenderingComponents.h"
#include "../components/MotionComponents.h"
#include "../ECS.h"
#include <math.h>

void planetaryAligntmentSystem(float dt)
{
  CPlanetaryAlignment* components_planetary_alignment = (CPlanetaryAlignment*)microECSComponentsGet(cid_planetary_alignment);
  const unsigned int components_count = microECSComponentsCount(cid_planetary_alignment);

  for (int i = 0; i < components_count; i++) {
    const int entityId = microECSComponentGetEntityId(cid_planetary_alignment, i);
    CPosition* position = (CPosition*)microECSEntityGetComponent(entityId, cid_position);
    CTransform* transform = (CTransform*)microECSEntityGetComponent(entityId, cid_transform);
    
    // Update rotation based on planet position
    float outVecX = position->x - components_planetary_alignment[i].planet_x;
    float outVecY = position->y - components_planetary_alignment[i].planet_y;
    float outVecLen = sqrt(outVecX * outVecX + outVecY * outVecY);
    outVecX /= outVecLen;
    outVecY /= outVecLen;
    float rotation = atan2(outVecY, outVecX) * 180.0 / M_PI + 90.0;
    transform->rotation = rotation;
  }
}
