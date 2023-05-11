#ifndef ECS_H
#define ECS_H

// Entity
extern int microECSEntityNew();
extern void microECSEntityFree(int entityId);
extern int microECSEntityIsAlive(int entityId);
extern void microECSEntityAddComponent(int entityId, const int componentTypeId, void* data);
extern void microECSEntityRemoveComponent(int entityId, const int componentTypeId);
extern void* microECSEntityGetComponent(int entityId, const int componentTypeId);

// Component
extern int microECSComponentRegister(int size);

// System
extern int microECSSystemAdd(void (*system)(float dt));
extern void microECSSystemRemove(int systemId);

// World
extern void microECSInit();
extern void microECSAllocateComponents();
extern void microECSFreeComponents();
extern void microECSRun(float dt);

// Query
typedef struct {
  int* entityIds;
  int size;
} ecs_component_query;
extern ecs_component_query microECSQueryByComponent(int componentTypeId);
extern ecs_component_query microECSQueryByComponents(int* componentTypeIds, int size);

#endif /* end of include guard: ECS_H */
