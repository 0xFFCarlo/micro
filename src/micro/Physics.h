#ifndef PHYSICS_H
#define PHYSICS_H

extern int microPhysicsWorldNew();
extern void microPhysicsWorldStep(int worldId, float dt);
extern int microPhysicsWorldsCount();
extern int microPhysicsWorldGetBodyCount(int worldId);
extern void microPhysicsWorldFree(int worldId);
extern void microPhysicsWorldFreeAll();

extern int microPhysicsBodyNewCircle(int worldId, float cx, float cy, float radius, float mass, unsigned char isStatic, float moment, float elasticity, float friction);
extern int microPhysicsBodyNewRect(int worldId, float cx, float cy, float width, float height, float mass, unsigned char isStatic, float elasticity);
extern void microPhysicsBodyFree(int bodyId);
extern int microPhysicsBodiesCount();
extern void microPhysicsBodySetMass(int bodyId, float mass);
extern float microPhysicsBodyGetMass(int bodyId);
extern void microPhysicsBodySetPosition(int bodyId, float x, float y);
extern void microPhysicsBodySetVelocity(int bodyId, float x, float y);
extern void microPhysicsBodySetForce(int bodyId, float x, float y);
extern void microPhysicsBodyGetPosition(int bodyId, float* x, float* y);
extern void microPhysicsBodyGetVelocity(int bodyId, float* x, float* y);
extern void microPhysicsBodyGetForce(int bodyId, float* x, float* y);

#endif /* end of include guard: PHYSICS_H */
