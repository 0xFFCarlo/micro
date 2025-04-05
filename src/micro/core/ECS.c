#include "ECS.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 2048
#define MAX_SYSTEMS 32
#define MAX_QUERIES 32

#define SETBITS(x, n) x |= n
#define CLEARBITS(x, n) x &= ~n
#define GETBIT(x, n) ((x >> n) & 1)
#define ENTITY_ALIVE_MASK 0x1
#define ENTITY_FREED_MASK 0x2

////////////////////////////////////
////// 128-BIT INTEGER TYPE ////////
////////////////////////////////////
typedef struct
{
  uint64_t low;
  uint64_t high;
} uint128_t;

uint128_t uint128_and_op(uint128_t a, uint128_t b)
{
  uint128_t result;
  result.low = a.low & b.low;
  result.high = a.high & b.high;
  return result;
}

void uint128_set_bit(uint128_t *a, int bit_pos, int value)
{
  if (bit_pos < 64)
  {
    a->low = a->low | (value << bit_pos);
    return;
  }
  a->high = a->high | (value << (bit_pos - 64));
}

int uint128_get_bit(uint128_t *a, int bit_pos)
{
  if (bit_pos < 64)
    return (a->low >> bit_pos) & 1;
  return (a->high >> (bit_pos - 64)) & 1;
}

int uint128_are_bits_set(uint128_t *a, uint128_t *mask)
{
  return (a->low & mask->low) == mask->low &&
         (a->high & mask->high) == mask->high;
}

uint128_t uint128(unsigned long long val)
{
  uint128_t result;
  result.low = val;
  result.high = 0;
  return result;
}

////////////////////////////////////
////// DATA STRUCTURES /////////////
////////////////////////////////////
typedef struct
{
  int id;
  uint8_t state;
  uint128_t components;
  uint16_t *component_offset;
  void *data;
  void (*free_entity)(int);
} entity_desc;
static entity_desc entities[MAX_ENTITIES];
static uint32_t entities_len = 0;
static int freed_entities[MAX_ENTITIES];
static int freed_entities_count = 0;
static Vector entities_to_remove;

// Component
typedef struct
{
  int component_size;
  void (*freeComponent)(void *);
} component_desc;
static component_desc components_types_desc[MAX_COMPONENTS];
static unsigned int components_types_count = 0;
static void *components[MAX_COMPONENTS];
static uint16_t components_count[MAX_COMPONENTS];
static uint16_t components_entity_ref[MAX_COMPONENTS][MAX_ENTITIES];
static int is_components_data_allocated = 0;

// System
static MicroECSSystem systems[MAX_SYSTEMS];
static unsigned int systems_count = 0;

// Query
typedef struct
{
  ecs_entity_list entities;
  uint128_t componentsMask;
  int (*compare)(int, int);
} ecs_cached_query;
static ecs_cached_query cached_queries[MAX_QUERIES];
static unsigned int cached_queries_count = 0;
static int freed_queries[MAX_QUERIES];
static int freed_queries_count = 0;
static unsigned int queries_comparisons_count = 0;

////////////////////////////////////
////// QUERY IMPLEMENTATION ////////
////////////////////////////////////
void insertion_sort(int arr[], int n, int (*compare)(int, int))
{
  for (int i = 1; i < n; i++)
  {
    int key = arr[i];
    int j = i - 1;
    while (j >= 0 && compare(arr[j], key) > 0)
    {
#ifdef DEBUG_MODE
      queries_comparisons_count++;
#endif
      arr[j + 1] = arr[j];
      j--;
    }
#ifdef DEBUG_MODE
    queries_comparisons_count++;
#endif
    arr[j + 1] = key;
  }
}

void microECSQuerySort(int queryId, int (*compare)(int, int))
{
  ecs_cached_query *query = &cached_queries[queryId];
  int *entityIds = query->entities.entityIds;
  int size = query->entities.size;
  insertion_sort(entityIds, size, compare);
}

void microECSQueriesEntityRemoved(int entityId)
{
  for (unsigned int i = 0; i < cached_queries_count; i++)
  {
    ecs_cached_query *query = &cached_queries[i];

    if (uint128_are_bits_set(&entities[entityId].components,
                             &query->componentsMask) == 0)
      continue;

    for (int j = 0; j < query->entities.size; j++)
    {
      if (query->entities.entityIds[j] == entityId)
      {
        query->entities.entityIds[j] = query->entities
                                         .entityIds[query->entities.size - 1];
        query->entities.size--;
        assert(query->entities.size >= 0);
        break;
      }
    }
  }
}

void microECSQueriesEntityChanged(int entityId, uint128_t old_components)
{
  for (unsigned int i = 0; i < cached_queries_count; i++)
  {
    ecs_cached_query *query = &cached_queries[i];

    int is_in = uint128_are_bits_set(&entities[entityId].components,
                                     &query->componentsMask);
    int was_in = uint128_are_bits_set(&old_components, &query->componentsMask);
    if (is_in == was_in)
      continue;

    if (is_in) // Add entity to query
    {
      query->entities.entityIds[query->entities.size] = entityId;
      query->entities.size++;
    }
    else // Remove entity from query
    {
      for (int j = 0; j < query->entities.size; j++)
      {
        if (query->entities.entityIds[j] == entityId)
        {
          query->entities.entityIds[j] = query->entities
                                           .entityIds[query->entities.size - 1];
          query->entities.size--;
          assert(query->entities.size >= 0);
          break;
        }
      }
    }
  }
}

////////////////////////////////////
////// ENTITY IMPLEMENTATION ///////
////////////////////////////////////
int microECSEntityNew(void *data, void (*free)(int))
{
  int id = -1;
  if (freed_entities_count > 0)
  {
    id = freed_entities[freed_entities_count - 1];
    freed_entities_count--;
  }
  else
  {
    assert(entities_len < MAX_ENTITIES - 1);
    id = entities_len;
    entities_len++;
  }

  entities[id].id = id;
  SETBITS(entities[id].state, ENTITY_ALIVE_MASK);
  CLEARBITS(entities[id].state, ENTITY_FREED_MASK);
  entities[id].free_entity = free;
  entities[id].data = data;
  entities[id].components = uint128(0); // No components enabled
  entities[id].component_offset = malloc(sizeof(uint16_t) *
                                         components_types_count);

  return id;
}

void microECSEntityRemove(int entityId)
{
  CLEARBITS(entities[entityId].state, ENTITY_ALIVE_MASK);
  vector_push_back(&entities_to_remove, &entityId);
}

void microECSEntityFree(int entityId)
{
  if (entities[entityId].state & ENTITY_FREED_MASK)
    return;

  // Remove from queries
  microECSQueriesEntityRemoved(entityId);

  // Free
  if (entities[entityId].free_entity != NULL)
    entities[entityId].free_entity(entityId);

  // Remove all components
  for (unsigned int i = 0; i < components_types_count; i++)
  {
    if (microECSEntityHasComponent(entityId, i))
      microECSEntityRemoveComponent(entityId, i);
  }

  // Free memory
  free(entities[entityId].component_offset);
  SETBITS(entities[entityId].state, ENTITY_FREED_MASK);

  // Add to freed entities
  freed_entities[freed_entities_count] = entityId;
  freed_entities_count++;
  assert(freed_entities_count < MAX_ENTITIES);
}

int microECSEntityIsAlive(int entityId)
{
  return (entities[entityId].state & ENTITY_ALIVE_MASK) != 0;
}

void *microECSEntityGetData(int entityId)
{
  return entities[entityId].data;
}

void microECSEntitySetData(int entityId, void *data)
{
  entities[entityId].data = data;
}

void microECSEntitySetFreeData(int entityId, void (*free)(int))
{
  entities[entityId].free_entity = free;
}

// Get component memory address of entity, without checking if it has it
// or if it is alive
static inline void *microECSEntityIndexComponent(int entityId,
                                                 const int componentTypeId)
{
  component_desc *cdesc = &components_types_desc[componentTypeId];
  const uint16_t component_offset = entities[entityId]
                                      .component_offset[componentTypeId];
  return components[componentTypeId] + component_offset * cdesc->component_size;
}

void microECSEntityAddComponent(int entityId, const int componentTypeId,
                                void *data)
{
  // Does entity have component already?
  assert(microECSEntityHasComponent(entityId, componentTypeId) == 0);

  // Get component description
  component_desc *cdesc = &components_types_desc[componentTypeId];
  uint128_t old_components = entities[entityId].components;

  // Update entity flags and offsets
  uint128_set_bit(&entities[entityId].components, componentTypeId, 1);
  entities[entityId]
    .component_offset[componentTypeId] = components_count[componentTypeId];
  assert(components_count[componentTypeId] < MAX_ENTITIES);

  // Store component and update entity reference
  components_count[componentTypeId]++;
  void *component = microECSEntityIndexComponent(entityId, componentTypeId);
  memcpy(component, data, cdesc->component_size);
  components_entity_ref[componentTypeId]
                       [components_count[componentTypeId] - 1] = entityId;

  // Update queries
  microECSQueriesEntityChanged(entityId, old_components);
}

void microECSEntityRemoveComponent(int entityId, const int componentTypeId)
{
  assert(entityId >= 0 && entityId < MAX_ENTITIES);
  assert(componentTypeId >= 0 && componentTypeId < MAX_COMPONENTS);

  // Upadte entity flags
  uint128_t old_components = entities[entityId].components;
  uint128_set_bit(&entities[entityId].components, componentTypeId, 0);

  // Free if necesary
  void *component = microECSEntityIndexComponent(entityId, componentTypeId);
  if (components_types_desc[componentTypeId].freeComponent != NULL)
    components_types_desc[componentTypeId].freeComponent(component);

  // Swap with last component to free memory
  if (entities[entityId].component_offset[componentTypeId] !=
      components_count[componentTypeId] - 1)
  {
    unsigned int component_size = components_types_desc[componentTypeId]
                                    .component_size;
    void *last_component = components[componentTypeId] +
                           (components_count[componentTypeId] - 1) *
                             component_size;
    memcpy(component, last_component, component_size);

    // Update entity offsets and entity ref
    int last_entity_id = components_entity_ref
      [componentTypeId][components_count[componentTypeId] - 1];
    entities[last_entity_id]
      .component_offset[componentTypeId] = entities[entityId]
                                             .component_offset[componentTypeId];
    components_entity_ref
      [componentTypeId]
      [entities[entityId].component_offset[componentTypeId]] = last_entity_id;
  }
  components_count[componentTypeId]--;

  // Update queries, if entity is alive but changed
  if (entities[entityId].state & ENTITY_ALIVE_MASK)
    microECSQueriesEntityChanged(entityId, old_components);
}

void *microECSEntityGetComponent(int entityId, const int componentTypeId)
{
#ifdef DEBUG_MODE
  if (microECSEntityHasComponent(entityId, componentTypeId) == 0 ||
      (entities[entityId].state & ENTITY_FREED_MASK) != 0 ||
      componentTypeId < 0 || componentTypeId >= MAX_COMPONENTS ||
      entityId < 0 || entityId >= MAX_ENTITIES)
  {
    debug_print("Something went wrong with entity %d and component %d\n",
                entityId, componentTypeId);
    return NULL;
  }
#endif
  return microECSEntityIndexComponent(entityId, componentTypeId);
}

int microECSEntityHasComponent(int entityId, const int componentTypeId)
{
  return uint128_get_bit(&entities[entityId].components, componentTypeId);
}

void microECSEntityFreeAll()
{
  for (unsigned int i = 0; i < entities_len; i++)
    if (entities[i].state & ENTITY_ALIVE_MASK)
      microECSEntityFree(i);
}

// Component
int microECSComponentRegister(int size, void (*freeComponent)(void *))
{
  assert(components_types_count < MAX_COMPONENTS);
  int id = components_types_count;
  components_types_count++;
  components_types_desc[id].component_size = size;
  components_types_desc[id].freeComponent = freeComponent;
  return id;
}

void *microECSComponentsGet(int componentTypeId)
{
  return components[componentTypeId];
}

int microECSComponentsCount(int componentTypeId)
{
  return components_count[componentTypeId];
}

int microECSComponentGetEntityId(int componentTypeId, int index)
{
  return components_entity_ref[componentTypeId][index];
}

int microECSEntitiesCount()
{
  return entities_len - freed_entities_count;
}

////////////////////////////////////
////// SYSTEM IMPLEMENTATION ///////
////////////////////////////////////
int microECSSystemAdd(MicroECSSystem system)
{
  for (unsigned int i = 0; i < systems_count; i++)
  {
    if (systems[i].update != NULL)
      continue;
    systems[i] = system;
    return i;
  }

  assert(systems_count < MAX_SYSTEMS);
  int id = systems_count;
  systems[id] = system;
  systems_count++;

  return id;
}

void microECSSystemRemove(int systemId)
{
  systems[systemId].update = NULL;
}

//////////////////////////////////////////
////// ENTITY SYSTEM IMPLEMENTATION //////
//////////////////////////////////////////
int microECSInit()
{
  // Initialize entity system
  for (int i = 0; i < MAX_ENTITIES; i++)
    freed_entities[i] = -1;
  entities_to_remove = vector_create(sizeof(int));
  return 0;
}

void microECSAllocateComponents()
{
  assert(is_components_data_allocated == 0);

  // allocate components vectors
  unsigned int memory_used = 0;
  for (unsigned int i = 0; i < components_types_count; i++)
  {
    const unsigned int component_size = components_types_desc[i].component_size;
    components[i] = malloc(component_size * MAX_ENTITIES);
    components_count[i] = 0;
    memory_used += component_size * MAX_ENTITIES;
  }
  is_components_data_allocated = 1;

  unsigned long memory = memory_used;
  memory += sizeof(components_count);
  memory += sizeof(components_entity_ref);
  memory += sizeof(entities);
  memory += sizeof(freed_entities);
  memory += sizeof(systems);
  memory += sizeof(cached_queries);
  memory += sizeof(components_types_desc);
  memory += sizeof(freed_entities_count);
  memory += sizeof(entities_len);
  memory += sizeof(systems_count);
  memory += sizeof(cached_queries_count);
  memory += sizeof(components_types_count);
  memory += sizeof(is_components_data_allocated);

  debug_print("microECS allocated %d kb for %d components\n",
              (int)(memory / (double)1024), components_types_count);
}

void microECSFree()
{
  // Remove entities that were marked for removal
  while (entities_to_remove.size)
  {
    const int eid = *(int *)vector_back(&entities_to_remove);
    vector_pop_back(&entities_to_remove);
    microECSEntityFree(eid);
  }

  // Free entities
  microECSEntityFreeAll();
  microECSCachedQueryFreeAll();
  if (entities_to_remove.data != NULL)
    vector_free(&entities_to_remove);

  // Free components
  if (is_components_data_allocated == 1)
  {
    // Free components vectors
    for (unsigned int i = 0; i < components_types_count; i++)
      free(components[i]);

    is_components_data_allocated = 0;
  }
}

void microECSRun(float dt)
{
  // Remove entities that were marked for removal
  while (entities_to_remove.size)
  {
    const int eid = *(int *)vector_back(&entities_to_remove);
    vector_pop_back(&entities_to_remove);
    microECSEntityFree(eid);
  }

  // Update systems
  for (unsigned int i = 0; i < systems_count; i++)
    if (systems[i].update != NULL)
      systems[i].update(dt);
}

int microECSGetDeletedEntitiesCount()
{
  return entities_to_remove.size;
}

void microECSGetDeletedEntities(int *entities, int *size)
{
  *size = entities_to_remove.size;
  memcpy(entities, entities_to_remove.data, sizeof(int) * entities_to_remove.size);
}

///////////////////////////////////
////// QUERY IMPLEMENTATION ///////
///////////////////////////////////
int microECSCachedQueryCreate(int *componentTypeIds, int size,
                              int (*sort_compare)(int, int))
{
  assert(cached_queries_count < MAX_QUERIES - 1);
  int query_id;
  if (freed_queries_count > 0)
  {
    query_id = freed_queries[freed_queries_count - 1];
    freed_queries_count--;
  }
  else
  {
    query_id = cached_queries_count;
    cached_queries_count++;
  }
  ecs_cached_query *query = &cached_queries[query_id];
  query->componentsMask = uint128(0);
  for (int i = 0; i < size; i++)
    uint128_set_bit(&query->componentsMask, componentTypeIds[i], 1);
  query->entities.entityIds = malloc(sizeof(int) * MAX_ENTITIES);
  query->entities.size = 0;
  query->compare = sort_compare;

  query->entities.size = 0;
  for (unsigned int i = 0; i < entities_len; i++)
  {
    if ((entities[i].state & ENTITY_ALIVE_MASK) == 0)
      continue;

    if (uint128_are_bits_set(&entities[i].components, &query->componentsMask))
    {
      query->entities.entityIds[query->entities.size] = i;
      query->entities.size++;
    }
  }

  // Sort entities
  if (query->compare != NULL)
    insertion_sort(query->entities.entityIds, query->entities.size,
                   query->compare);

  return query_id;
}

void microECSCachedQueryFree(int queryId)
{
  ecs_cached_query *query = &cached_queries[queryId];
  free(query->entities.entityIds);
  query->entities.entityIds = NULL;
  query->entities.size = 0;
  query->compare = NULL;
  freed_queries[freed_queries_count] = queryId;
  freed_queries_count++;
}

ecs_entity_list microECSCachedQueryRun(int queryId)
{
  ecs_cached_query *query = &cached_queries[queryId];
  if (query->compare != NULL)
    microECSQuerySort(queryId, query->compare);
  return query->entities;
}

void microECSCachedQueryFreeAll()
{
  for (unsigned int i = 0; i < cached_queries_count; i++)
    microECSCachedQueryFree(i);
}

unsigned int microECSQueriesComparisions()
{
  return queries_comparisons_count;
}

void microECSQueriesResetComparisions()
{
  queries_comparisons_count = 0;
}
