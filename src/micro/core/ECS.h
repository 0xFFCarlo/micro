#ifndef ECS_H
#define ECS_H

////////////////////////////////
// --- Entity ---
////////////////////////////////

// Create new entity and return its id.
int microECSEntityNew(void *data, void (*free)(int));

// Request to remove entity from the world.
// It will be removed at the next iteration of microECSRun().
void microECSEntityQueueFree(int entityId);

// Check if entity is alive.
int microECSEntityIsAlive(int entityId);

// Get entity private data pointer.
void *microECSEntityGetData(int entityId);

// Set entity private data pointer.
void microECSEntitySetData(int entityId, void *data);

// Set entity free data function.
void microECSEntitySetFreeData(int entityId, void (*free)(int));

// Add component to entity.
void microECSEntityAddComponent(int entityId, const int componentTypeId,
                                void *data);

// Remove component from entity.
void microECSEntityRemoveComponent(int entityId, const int componentTypeId);

// Get component owned by entity.
void *microECSEntityGetComponent(int entityId, const int componentTypeId);

// Check if entity has component.
int microECSEntityHasComponent(int entityId, const int componentTypeId);

// Get number of entities in the world.
int microECSEntitiesCount();

// Free all entities.
void microECSEntityFreeAll();

////////////////////////////////
// --- Component ---
////////////////////////////////

// Register new component type and return its id.
int microECSComponentRegister(int size, void (*freeComponent)(void *));

// Get components array by component type id.
void *microECSComponentsGet(int componentTypeId);

// Get number of components by component type id.
int microECSComponentsCount(int componentTypeId);

// Get entity id that owns component
int microECSComponentGetEntityId(int componentTypeId, int index);

////////////////////////////////
// --- System ---
////////////////////////////////

typedef struct MicroECSSystem
{
  void (*update)(float dt);
  void (*entity_add)(int entityId);
  void (*entity_remove)(int entityId);
} MicroECSSystem;

// Register new system and return its id.
int microECSSystemAdd(MicroECSSystem system);

// Remove system by id.
void microECSSystemRemove(int systemId);

////////////////////////////////
// --- World ---
////////////////////////////////

// Initialize world.
int microECSInit();

// Allocate memory for components.
void microECSAllocateComponents();

// Free memory for components.
void microECSFree();

// Run all systems.
void microECSRun(float dt);

// Get number of deleted entities in the last frame.
int microECSGetDeletedEntitiesCount();

// Return the entities ids that were deleted in the last frame.
void microECSGetDeletedEntities(int *entities, int *size);

////////////////////////////////
// --- Query ---
////////////////////////////////
typedef struct
{
  int *entityIds;
  int size;
} ecs_entity_list;

// Create new query and return its id.
int microECSCachedQueryCreate(int *componentTypeIds, int size,
                              int (*sort_compare)(int, int));

// Free query.
void microECSCachedQueryFree(int queryId);

// Run query and return list of entities.
ecs_entity_list microECSCachedQueryRun(int queryId);

// Free all queries.
void microECSCachedQueryFreeAll();

// Get number of comparisions in sorting queries.
unsigned int microECSQueriesComparisions();

// Reset number of comparisions in sorting queries.
void microECSQueriesResetComparisions();

#endif /* end of include guard: ECS_H */
