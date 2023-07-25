#include "GravitySystem.h"
#include "../components/CustomComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include <math.h>
#include <stdio.h>

void gravitySystem(float dt)
{
  (void)(dt); // Unused parameter

  CGravity *components_gravity = (CGravity *)microECSComponentsGet(cid_gravity);
  const unsigned int components_count = microECSComponentsCount(cid_gravity);

  for (unsigned int i = 0; i < components_count; i++)
  {
    const int entityId = microECSComponentGetEntityId(cid_gravity, i);
    CPosition *position = (CPosition *)microECSEntityGetComponent(entityId,
                                                                  cid_position);
    CBody *body = (CBody *)microECSEntityGetComponent(entityId, cid_body);
    CGravity *gravity = &components_gravity[i];

    // Update gravity based on planet position
    float dx = gravity->x - position->x;
    float dy = gravity->y - position->y;
    float dist = sqrt(dx * dx + dy * dy);
    float mass = microPhysicsBodyGetMass(body->body_id);
    float force = mass / (dist * dist);
    float fx = force * dx / dist;
    float fy = force * dy / dist;
    microPhysicsBodySetForce(body->body_id, fx, fy);
  }
}
