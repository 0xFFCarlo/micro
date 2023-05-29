#include <stdlib.h>
#include "PhysicsSystem.h"
#include "../components/MotionComponents.h"
#include "../ECS.h"
#include <math.h>

int check_collision(float x1, float y1, float x2, float y2, float r1, float r2)
{
  float dx = x1 - x2;
  float dy = y1 - y2;
  float distance_squared = dx * dx + dy * dy;
  return distance_squared < (r1 + r2) * (r1 + r2);
}


void physicsSystem(float dt)
{
  CBody* components_body = (CBody*)microECSComponentsGet(cid_body);
  const unsigned int components_count = microECSComponentsCount(cid_body);

  // Allocate arrays to store calculated magnitudes and directions
  float* magnitudes = malloc(components_count * sizeof(float));
  float* directions = malloc(components_count * sizeof(float));

  // Apply forces and integrate velocities 
  for (int i = 0; i < components_count; i++) {
    CBody* body = &components_body[i];
    const int entityId = microECSComponentGetEntityId(cid_body, i);
    CPosition* pos = (CPosition*)microECSEntityGetComponent(entityId, cid_position);

    body->velX += body->forceX * dt;
    body->velY += body->forceY * dt;
    pos->x += body->velX;
    pos->y += body->velY;

    body->forceX = 0;
    body->forceY = 0;

    // Compute magnitudes and directions
    magnitudes[i] = sqrt(body->velX * body->velX + body->velY * body->velY);
    directions[i] = atan2(body->velY, body->velX);
  }

  // Collision resolution
  for (int i = 0; i < components_count; i++) {
    CBody* body1 = &components_body[i];
    const int entityId1 = microECSComponentGetEntityId(cid_body, i);
    CPosition* pos1 = (CPosition*)microECSEntityGetComponent(entityId1, cid_position);

    for (int j = i + 1; j < components_count; j++) {
      CBody* body2 = &components_body[j];
      const int entityId2 = microECSComponentGetEntityId(cid_body, j);
      CPosition* pos2 = (CPosition*)microECSEntityGetComponent(entityId2, cid_position);

      if (check_collision(pos1->x, pos1->y, pos2->x, pos2->y, body1->radius, body2->radius)) {
        if (body1->isStatic && body2->isStatic) {
          continue;
        }
        else if (body1->isStatic && !body2->isStatic) 
        {
          // The moving body (body2) changes its velocity according to its restitution
          body2->velX = -body2->restitution * body2->velX;
          body2->velY = -body2->restitution * body2->velY;
        }
        else if (!body1->isStatic && body2->isStatic) 
        {
          // The moving body (body) changes its velocity according to its restitution
          body1->velX = -body1->restitution * body1->velX;
          body1->velY = -body1->restitution * body1->velY;
        }
        else 
        {
          float dx = pos1->x - pos2->x;
          float dy = pos1->y - pos2->y;
          float collision_angle = atan2(dy, dx);

          // Use the precomputed magnitudes and directions
          float magnitude1 = magnitudes[i];
          float magnitude2 = magnitudes[j];
          float direction1 = directions[i];
          float direction2 = directions[j];

          // calculate velocities along the normal and tangent lines
          float normal1 = magnitude1 * cos(direction1 - collision_angle);
          float normal2 = magnitude2 * cos(direction2 - collision_angle);
          float tangent1 = magnitude1 * sin(direction1 - collision_angle);
          float tangent2 = magnitude2 * sin(direction2 - collision_angle);

          // conserve tangential momentum
          float tangent_final1 = tangent1;
          float tangent_final2 = tangent2;

          // calculate the restitution value. For simplicity, we will take the minimum restitution value.
          float e = fmin(body1->restitution, body2->restitution);

          // calculate the final normal velocities including the restitution
          float normal_final1 = ((e * body2->mass * (normal2 - normal1)) + (body1->mass * normal1) + (body2->mass * normal2)) / (body1->mass + body2->mass);
          float normal_final2 = ((e * body1->mass * (normal1 - normal2)) + (body2->mass * normal2) + (body1->mass * normal1)) / (body1->mass + body2->mass);

          // convert back to regular velocities
          body1->velX = cos(collision_angle) * normal_final1 + cos(collision_angle + M_PI/2) * tangent_final1;
          body1->velY = sin(collision_angle) * normal_final1 + sin(collision_angle + M_PI/2) * tangent_final1;
          body2->velX = cos(collision_angle) * normal_final2 + cos(collision_angle + M_PI/2) * tangent_final2;
          body2->velY = sin(collision_angle) * normal_final2 + sin(collision_angle + M_PI/2) * tangent_final2;
        }
      }
    }
  }

  free(magnitudes);
  free(directions);
}
