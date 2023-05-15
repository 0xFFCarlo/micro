#include "ECS.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 1024
#define MAX_SYSTEMS 32
#define MAX_QUERIES 32

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

uint128_t uint128_and_op(uint128_t a, uint128_t b) {
    uint128_t result;
    result.low = a.low & b.low;
    result.high = a.high & b.high;
    return result;
}

void uint128_set_bit(uint128_t* a, int bit_pos, int value) {
  if (bit_pos < 64) {
    a->low = a->low | (value << bit_pos);
    return;
  }
  a->high = a->high | (value << (bit_pos - 64));
}

int uint128_get_bit(uint128_t* a, int bit_pos) {
  if (bit_pos < 64)
    return (a->low >> bit_pos) & 1;
  return (a->high >> (bit_pos - 64)) & 1;
}

int uint128_are_bits_set(uint128_t* a, uint128_t* mask) {
  return (a->low & mask->low) == mask->low && (a->high & mask->high) == mask->high;
}

uint128_t uint128(unsigned long long val) {
    uint128_t result;
    result.low = val;
    result.high = 0;
    return result;
}

// Entity
typedef struct {
  int id;
  uint8_t alive;
  uint128_t components;
  void* data;
  void (*update)(void*);
  void (*free_entity)(void*);
} entity_desc;
entity_desc entities[MAX_ENTITIES];
unsigned int entities_count = 0; 
int freed_entities[MAX_ENTITIES];
int freed_entities_count = 0;
int query_buf[MAX_ENTITIES];

// Component
typedef struct {
  int component_size;
  int memory_offset;
  void (*free_component)(void*);
} component_desc;
component_desc components_types_desc[MAX_COMPONENTS];
unsigned int components_types_count = 0;
void* components_data = NULL;
unsigned long is_components_data_allocated = 0;


// System
typedef struct {
  void (*system)(float);
} system_desc;
system_desc systems[MAX_SYSTEMS];
unsigned int systems_count = 0;


// Query
typedef struct {
  ecs_entity_list entities;
  uint128_t componentsMask;
  uint8_t updated; 
  int(*compare)(int, int);
} ecs_query;
ecs_query queries[MAX_QUERIES];
unsigned int queries_count = 0;
int freed_queries[MAX_QUERIES];
int freed_queries_count = 0;


void insertion_sort(int arr[], int n, int(*compare)(int, int)) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && compare(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void microECSQuerySort(int queryId, int(*compare)(int, int))
{
  ecs_query* query = &queries[queryId];
  int* entityIds = query->entities.entityIds;
  int size = query->entities.size;
  insertion_sort(entityIds, size, compare);
}

void microECSQueriesEntityRemoved(int entityId)
{
  for (int i = 0; i < queries_count; i++) {
    ecs_query* query = &queries[i];

    if (uint128_are_bits_set(&entities[entityId].components, &query->componentsMask) == 0)
      continue;

    for (int j = 0; j < query->entities.size; j++) {
      if (query->entities.entityIds[j] == entityId) {
        query->entities.entityIds[j] = query->entities.entityIds[query->entities.size-1];
        query->entities.size--;
        break;
      }
    }

    if (query->compare != NULL)
      microECSQuerySort(i, query->compare);
  }
}

void microECSQueriesEntityChanged(int entityId, uint128_t old_components)
{
  for (int i = 0; i < queries_count; i++) {
    ecs_query* query = &queries[i];

    int is_in = uint128_are_bits_set(&entities[entityId].components, &query->componentsMask);
    int was_in = uint128_are_bits_set(&old_components, &query->componentsMask);
    if (is_in == was_in) continue;

    if (is_in == 1) // Add entity to query
    { 
      query->entities.entityIds[query->entities.size] = entityId;
      query->entities.size++;
      if (query->compare != NULL)
        microECSQuerySort(i, query->compare);
    } 
    else // Remove entity from query
    { 
      for (int j = 0; j < query->entities.size; j++) {
        if (query->entities.entityIds[j] == entityId) {
          query->entities.entityIds[j] = query->entities.entityIds[query->entities.size-1];
          query->entities.size--;
          break;
        }
      }
      if (query->compare != NULL)
        microECSQuerySort(i, query->compare);
    }
  }
}


// Entity
int microECSEntityNew(void* data, void(*update)(void*), void(*free)(void*))
{
  int id = -1;
  if (freed_entities_count > 0) {
    id = freed_entities[freed_entities_count - 1];
    freed_entities_count--;
  }
  else
  {
    assert(entities_count < MAX_ENTITIES-1);
    id = entities_count;
    entities_count++;
  }

  entities[id].id = id;
  entities[id].alive = 1;
  entities[id].free_entity = free;
  entities[id].update = update;
  entities[id].data = data;

  //Disable all components
  for (int i = 0; i < MAX_COMPONENTS; i++)
    entities[id].components = uint128(0);

  return id;
}

void microECSEntityFree(int entityId)
{
  microECSQueriesEntityRemoved(entityId);
  entities[entityId].alive = 0;
  if (entities[entityId].free_entity != NULL)
    entities[entityId].free_entity(NULL);
  freed_entities[freed_entities_count] = entityId;
  freed_entities_count++;
  assert(freed_entities_count < MAX_ENTITIES);
}

int microECSEntityIsAlive(int entityId)
{
  return entities[entityId].alive;
}

void microECSEntityAddComponent(int entityId, const int componentTypeId, void* data)
{
  component_desc* cdesc = &components_types_desc[componentTypeId];
  uint128_t old_components = entities[entityId].components;
  uint128_set_bit(&entities[entityId].components, componentTypeId, 1);
  void* component = microECSEntityGetComponent(entityId, componentTypeId);
  memcpy(component, data, cdesc->component_size);
  microECSQueriesEntityChanged(entityId, old_components);
}

void microECSEntityRemoveComponent(int entityId, const int componentTypeId)
{
  uint128_t old_components = entities[entityId].components;
  uint128_set_bit(&entities[entityId].components, componentTypeId, 0);
  if (components_types_desc[componentTypeId].free_component != NULL) {
    void* component = microECSEntityGetComponent(entityId, componentTypeId);
    components_types_desc[componentTypeId].free_component(component);
  }
  microECSQueriesEntityChanged(entityId, old_components);
}

void* microECSEntityGetComponent(int entityId, const int componentTypeId)
{
  component_desc* cdesc = &components_types_desc[componentTypeId];
  return components_data + cdesc->memory_offset + entityId * cdesc->component_size;
}

int microECSEntityHasComponent(int entityId, const int componentTypeId)
{
  return uint128_get_bit(&entities[entityId].components, componentTypeId);
}


// Component
int microECSComponentRegister(int size, void (*free)(void*))
{
  assert(components_types_count < MAX_COMPONENTS);
  int id = components_types_count;
  components_types_count++;

  components_types_desc[id].component_size = size;
  components_types_desc[id].free_component = free;
  if (id == 0)
    components_types_desc[id].memory_offset = 0;
  else
    components_types_desc[id].memory_offset = components_types_desc[id-1].memory_offset + components_types_desc[id-1].component_size * MAX_ENTITIES;

  return id;
}



// System
int microECSSystemAdd(void (*system)(float dt))
{
  for (int i = 0; i < systems_count; i++) {
    if (systems[i].system == NULL) {
      systems[i].system = system;
      return i;
    }
  }

  assert(systems_count < MAX_SYSTEMS-1);
  int id = systems_count;
  systems[id].system = system;
  systems_count++;

  return id;
}

void microECSSystemRemove(int systemId)
{
  systems[systemId].system = NULL;
}


// World
void microECSInit()
{
  // Initialize entity system
  for (int i = 0; i < MAX_ENTITIES; i++)
    freed_entities[i] = -1;

  components_data = NULL;
  is_components_data_allocated = 0;
}

void microECSAllocateComponents()
{
  //compute entity size
  unsigned int component_offset = 0;
  for (int i = 0; i < components_types_count; i++) {
    components_types_desc[i].memory_offset = component_offset;
    component_offset += components_types_desc[i].component_size * MAX_ENTITIES;
  }

  //allocate entity data
  if (is_components_data_allocated == 0) {
    components_data = malloc(component_offset);
    is_components_data_allocated = 1;
  }

  unsigned long memory = component_offset;
  memory += sizeof(entities);
  memory += sizeof(freed_entities);
  memory += sizeof(systems);
  memory += sizeof(queries);
  memory += sizeof(query_buf);
  memory += sizeof(components_types_desc);
  memory += sizeof(freed_entities_count);
  memory += sizeof(entities_count);
  memory += sizeof(systems_count);
  memory += sizeof(queries_count);
  memory += sizeof(components_types_count);
  memory += sizeof(is_components_data_allocated);

  printf("microECS allocated %d kb\n", (int)(memory / (double)1024));
  printf("microECS components: %d", components_types_count);
}

void microECSFree()
{
  if (is_components_data_allocated == 1) {
    free(components_data);
    is_components_data_allocated = 0;
    components_data = NULL;
  }
}

void microECSRun(float dt)
{
  //Update entities
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 1 && entities[i].update != NULL) {
      entities[i].update(entities[i].data);
    }
  }
  
  //Update systems
  for (int i = 0; i < systems_count; i++) {
    if (systems[i].system != NULL) {
      systems[i].system(dt);
    }
  }
}

ecs_entity_list microECSEntityByComponent(int componentTypeId)
{
  ecs_entity_list query;
  query.entityIds = &query_buf[0];
  query.size = 0;
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 1 && uint128_get_bit(&entities[i].components, componentTypeId) == 1) {
      query.entityIds[query.size] = i;
      query.size++;
    }
  }
  return query;
}

ecs_entity_list microECSEntityByComponents(int* componentTypeIds, int size)
{
  ecs_entity_list query;
  query.entityIds = &query_buf[0];
  query.size = 0;
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 0) continue;
    int found = 1;
    for (int j = 0; j < size; j++) {
      if (uint128_get_bit(&entities[i].components, componentTypeIds[j]) == 0) {
        found = 0;
        break;
      }
    }
    if (found == 1) {
      query.entityIds[query.size] = i;
      query.size++;
    }
  }
  return query;
}

// Query
int microECSQueryCreate(int* componentTypeIds, int size, int(*sort_compare)(int, int))
{
  assert(queries_count < MAX_QUERIES-1);
  int query_id;
  if (freed_queries_count > 0) {
    query_id = freed_queries[freed_queries_count-1];
    freed_queries_count--;
  } else {
    query_id = queries_count;
    queries_count++;
  }
  ecs_query* query = &queries[query_id];
  query->componentsMask = uint128(0);
  for (int i = 0; i < size; i++)
    uint128_set_bit(&query->componentsMask, componentTypeIds[i], 1);
  query->entities.entityIds = malloc(sizeof(int) * MAX_ENTITIES);
  query->entities.size = 0;
  query->updated = 0;
  query->compare = sort_compare;
  return query_id;
}

void microECSQueryFree(int queryId)
{
  ecs_query* query = &queries[queryId];
  free(query->entities.entityIds);
  query->entities.entityIds = NULL;
  query->entities.size = 0;
  query->compare = NULL;
  freed_queries[freed_queries_count] = queryId;
  freed_queries_count++;
}

ecs_entity_list microECSQueryRun(int queryId)
{
  ecs_query* query = &queries[queryId];
  if (query->updated) return query->entities;
  
  query->entities.size = 0;
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 0) continue;
    if (uint128_are_bits_set(&entities[i].components, &query->componentsMask)) {
      query->entities.entityIds[query->entities.size] = i;
      query->entities.size++;
    }
  }

  // Sort entities
  if (query->compare != NULL)
    insertion_sort(query->entities.entityIds, query->entities.size, query->compare);

  query->updated = 1;
  return query->entities;
}
