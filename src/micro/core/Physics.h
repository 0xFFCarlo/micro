#ifndef PHYSICS_H
#define PHYSICS_H

#include "../util/Types.h"

int microPhysicsWorldNew();
void microPhysicsWorldStep(int worldId, float dt);
int microPhysicsWorldsCount();
int microPhysicsWorldGetBodyCount(int worldId);
void microPhysicsWorldFree(int worldId);
void microPhysicsWorldFreeAll();

int microPhysicsBodyNewCircle(int entityId, int worldId, float cx,
                                     float cy, float radius, float mass,
                                     u8 isStatic, u8 canRotate,
                                     float elasticity, float friction);
int microPhysicsBodyNewRect(int entityId, int worldId, float cx,
                                   float cy, float width, float height,
                                   float mass, u8 isStatic, u8 canRotate,
                                   float elasticity, float friction);
void microPhysicsBodySetFilter(int bodyId, int category, u32 mask);
void microPhysicsBodySetCollisionCallback(int bodyId,
                                                 void (*callback)(int, int)); 
void microPhysicsBodyFree(int bodyId);
int microPhysicsBodiesCount();
void microPhysicsBodySetMass(int bodyId, float mass);
float microPhysicsBodyGetMass(int bodyId);
void microPhysicsBodySetPosition(int bodyId, float x, float y);
void microPhysicsBodySetVelocity(int bodyId, float x, float y);
void microPhysicsBodySetForce(int bodyId, float x, float y);
void microPhysicsBodyApplyForce(int bodyId, float x, float y);
void microPhysicsBodyGetPosition(int bodyId, float *x, float *y);
void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y);
void microPhysicsBodyGetForce(int bodyId, float *x, float *y);
void microPhysicsBodySetRotation(int bodyId, float angle);
float microPhysicsBodyGetRotation(int bodyId);
int microPhysicsBodyGetWorldId(int bodyId);

#endif /* end of include guard: PHYSICS_H */
