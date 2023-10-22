#ifndef ECS_H
#define ECS_H

////////////////////////////////
// --- Entity ---
////////////////////////////////

// Create new entity and return its id.
extern int microECSEntityNew(void *data, void (*free)(int));

// Request to remove entity from the world.
// It will be removed at the next iteration of microECSRun().
extern void microECSEntityRemove(int entityId);

// Check if entity is alive.
extern int microECSEntityIsAlive(int entityId);

// Get entity private data pointer.
extern void* microECSEntityGetData(int entityId);

// Add component to entity.
extern void microECSEntityAddComponent(int entityId, const int componentTypeId,
                                       void *data);

// Remove component from entity.
extern void microECSEntityRemoveComponent(int entityId,
                                          const int componentTypeId);

// Get component owned by entity.
extern void *microECSEntityGetComponent(int entityId,
                                        const int componentTypeId);

// Check if entity has component.
extern int microECSEntityHasComponent(int entityId, const int componentTypeId);

// Get number of entities in the world.
extern int microECSEntitiesCount();

// Free all entities.
extern void microECSEntityFreeAll();

////////////////////////////////
// --- Component ---
////////////////////////////////

// Register new component type and return its id.
extern int microECSComponentRegister(int size);

// Get components array by component type id.
extern void *microECSComponentsGet(int componentTypeId);

// Get number of components by component type id.
extern int microECSComponentsCount(int componentTypeId);

// Get entity id that owns component
extern int microECSComponentGetEntityId(int componentTypeId, int index);

////////////////////////////////
// --- System ---
////////////////////////////////

// Register new system and return its id.
extern int microECSSystemAdd(void (*system)(float dt));

// Remove system by id.
extern void microECSSystemRemove(int systemId);

////////////////////////////////
// --- World ---
////////////////////////////////

// Initialize world.
extern void microECSInit();

// Allocate memory for components.
extern void microECSAllocateComponents();

// Free memory for components.
extern void microECSFree();

// Run all systems.
extern void microECSRun(float dt);

////////////////////////////////
// --- Query ---
////////////////////////////////
typedef struct
{
  int *entityIds;
  int size;
} ecs_entity_list;

// Create new query and return its id.
extern int microECSCachedQueryCreate(int *componentTypeIds, int size,
                                     int (*sort_compare)(int, int));

// Free query.
extern void microECSCachedQueryFree(int queryId);

// Run query and return list of entities.
extern ecs_entity_list microECSCachedQueryRun(int queryId);

// Free all queries.
extern void microECSCachedQueryFreeAll();

#endif /* end of include guard: ECS_H */
