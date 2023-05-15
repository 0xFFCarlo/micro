#ifndef ECS_H
#define ECS_H

// Entity
extern int microECSEntityNew( void* data, void(*update)(void*), void(*free)(void*) );
extern void microECSEntityFree(int entityId);
extern int microECSEntityIsAlive(int entityId);
extern void microECSEntityAddComponent(int entityId, const int componentTypeId, void* data);
extern void microECSEntityRemoveComponent(int entityId, const int componentTypeId);
extern void* microECSEntityGetComponent(int entityId, const int componentTypeId);
extern int microECSEntityHasComponent(int entityId, const int componentTypeId);

typedef struct {
  int* entityIds;
  int size;
} ecs_entity_list;
extern ecs_entity_list microECSEntityByComponent(int componentTypeId);
extern ecs_entity_list microECSEntityByComponents(int* componentTypeIds, int size);


// Component
extern int microECSComponentRegister(int size, void (*free)(void*));


// System
extern int microECSSystemAdd(void (*system)(float dt));
extern void microECSSystemRemove(int systemId);


// World
extern void microECSInit();
extern void microECSAllocateComponents();
extern void microECSFree();
extern void microECSRun(float dt);


// Query
extern int microECSQueryCreate(int* componentTypeIds, int size, int(*sort_compare)(int, int));
extern void microECSQueryFree(int queryId);
extern ecs_entity_list microECSQueryRun(int queryId);


#endif /* end of include guard: ECS_H */
