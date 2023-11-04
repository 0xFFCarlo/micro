#ifndef PHYSICS_H
#define PHYSICS_H

#include "Types.h"

extern int microPhysicsWorldNew();
extern void microPhysicsWorldStep(int worldId, float dt);
extern int microPhysicsWorldsCount();
extern int microPhysicsWorldGetBodyCount(int worldId);
extern void microPhysicsWorldFree(int worldId);
extern void microPhysicsWorldFreeAll();

extern int microPhysicsBodyNewCircle(int entityId, int worldId, float cx,
                                     float cy, float radius, float mass,
                                     u8 isStatic, u8 canRotate,
                                     float elasticity, float friction);
extern int microPhysicsBodyNewRect(int entityId, int worldId, float cx,
                                   float cy, float width, float height,
                                   float mass, u8 isStatic, u8 canRotate,
                                   float elasticity, float friction);
extern void microPhysicsBodySetFilter(int bodyId, int category, u32 mask);
extern void microPhysicsBodySetCollisionCallback(int bodyId,
                                                 void (*callback)(int, int)); 
extern void microPhysicsBodyFree(int bodyId);
extern int microPhysicsBodiesCount();
extern void microPhysicsBodySetMass(int bodyId, float mass);
extern float microPhysicsBodyGetMass(int bodyId);
extern void microPhysicsBodySetPosition(int bodyId, float x, float y);
extern void microPhysicsBodySetVelocity(int bodyId, float x, float y);
extern void microPhysicsBodySetForce(int bodyId, float x, float y);
extern void microPhysicsBodyApplyForce(int bodyId, float x, float y);
extern void microPhysicsBodyGetPosition(int bodyId, float *x, float *y);
extern void microPhysicsBodyGetVelocity(int bodyId, float *x, float *y);
extern void microPhysicsBodyGetForce(int bodyId, float *x, float *y);
extern void microPhysicsBodySetRotation(int bodyId, float angle);
extern float microPhysicsBodyGetRotation(int bodyId);
extern int microPhysicsBodyGetWorldId(int bodyId);

#endif /* end of include guard: PHYSICS_H */
