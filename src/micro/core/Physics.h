#ifndef PHYSICS_H
#define PHYSICS_H

#include "../util/Types.h"

typedef struct
{
  int eid_a, eid_b;
} MicroCollisionPair;

typedef void (*MicroWorldCollisionsCb)(MicroCollisionPair *begins,
                                       int begins_count,
                                       MicroCollisionPair *updates,
                                       int updates_count);

typedef void (*MicroWorldBodiesRemovedCb)(const int *bodyIds, int bodyCount);

// Create a new physics world
int microPhysicsWorldNew(MicroWorldCollisionsCb collisions_callback,
                         MicroWorldBodiesRemovedCb bodies_removed_callback);

// Create a new physics world with a spatial hash
int microPhysicsWorldUseSpatialHash(int worldId, int cell_size, int cell_count);

// Do a single step of the physics simulation
void microPhysicsWorldStep(int worldId, float dt);

// Get the number of physics worlds
int microPhysicsWorldsCount();

// Get the number of bodies in a physics world
int microPhysicsWorldGetBodyCount(int worldId);

// Create a new collision tilemap in a physics world
int microPhysicsWorldNewCollisionTilemap(int worldId,
                                         bool (*is_solid)(const int px,
                                                          const int py),
                                         int tile_size);

// Free World
void microPhysicsWorldFree(int worldId);

// Free all worlds
void microPhysicsWorldFreeAll();

// Create a new physics body with a circle shape and add it to a world
int microPhysicsBodyNewCircle(int entityId, int worldId, float cx, float cy,
                              float radius, float mass, uint8_t isStatic,
                              uint8_t canRotate, float elasticity,
                              float friction);

// Create a new physics body with a rectangle shape and add it to a world
int microPhysicsBodyNewRect(int entityId, int worldId, float cx, float cy,
                            float width, float height, float mass,
                            uint8_t isStatic, uint8_t canRotate,
                            float elasticity, float friction);

// Set the collision filter of a physics body.
// Each bit of mask is a category id that the body will collide with.
void microPhysicsBodySetFilter(int bodyId, int category, uint32_t mask);

// Respond to a collision between two bodies
void microPhysicsBodySetCollisionBeginCallback(int bodyId,
                                               void (*callback)(int, int));

// Respond to a collision between two bodies
void microPhysicsBodySetCollisionUpdateCallback(int bodyId,
                                                void (*callback)(int, int));

// Set the collision tilemap of a physics body
void microPhysicsBodySetCollisionTilemap(int bodyId, int tilemapId);

// Free a physics body
void microPhysicsBodyFree(int bodyId);

// Set the mass of a physics body
void microPhysicsBodySetMass(int bodyId, float mass);

// Get the mass of a physics body
float microPhysicsBodyGetMass(int bodyId);

// Set the position of a physics body
void microPhysicsBodySetPosition(int bodyId, float x, float y);

// Set the velocity of a physics body
void microPhysicsBodySetVelocity(int bodyId, float x, float y);

// Set the force of a physics body
void microPhysicsBodySetForce(int bodyId, float x, float y);

// Apply a force to a physics body
void microPhysicsBodyApplyForce(int bodyId, float x, float y);

// Get the position of a physics body
void microPhysicsBodyGetPosition(int bodyId, float *x, float *y);

// Get the velocity of a physics body
void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y);

// Get the force of a physics body
void microPhysicsBodyGetForce(int bodyId, float *x, float *y);

// Set the rotation of a physics body
void microPhysicsBodySetRotation(int bodyId, float angle);

// Get the rotation of a physics body
float microPhysicsBodyGetRotation(int bodyId);

// Set if a physics body is a sensor
void microPhysicsBodySetSensor(int bodyId, bool is_sensor);

// Get if a physics body is a sensor
bool microPhysicsBodyIsSensor(int bodyId);

// Check if a physics body is static
bool microPhysicsBodyIsStatic(int bodyId);

// Check if a physics body id is valid
bool microPhysicsBodyIsValid(int bodyId);

// Get the world id of a physics body
int microPhysicsBodyGetWorldId(int bodyId);

// Get shape of a physics body
MicroAABB microPhysicsBodyGetAABB(int bodyId);

#endif /* end of include guard: PHYSICS_H */
