#include "ECS.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 1024
#define MAX_SYSTEMS 32

// Entity
typedef struct {
  int id;
  int alive;
  int components[MAX_COMPONENTS];
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
} component_desc;
component_desc components_types_desc[MAX_COMPONENTS];
unsigned int components_types_count = 0;
void* entity_data = NULL;
unsigned long entity_size = 0;
unsigned long is_entity_data_allocated = 0;


// System
typedef struct {
  void (*system)(float);
} system_desc;
system_desc systems[MAX_SYSTEMS];
unsigned int systems_count = 0;



// Entity
int microECSEntityNew()
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

  //Disable all components
  for (int i = 0; i < MAX_COMPONENTS; i++)
    entities[id].components[i] = -1;

  return id;
}

void microECSEntityFree(int entityId)
{
  entities[entityId].alive = 0;
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
  int component_id;
  component_desc* cdesc = &components_types_desc[componentTypeId];
  entities[entityId].components[componentTypeId] = 1; // Enable component
  memcpy(entity_data + entityId * entity_size + cdesc->memory_offset, data, cdesc->component_size);
}

void microECSEntityRemoveComponent(int entityId, const int componentTypeId)
{
  entities[entityId].components[componentTypeId] = -1;
}

void* microECSEntityGetComponent(int entityId, const int componentTypeId)
{
  component_desc* cdesc = &components_types_desc[componentTypeId];
  return entity_data + entityId * entity_size + cdesc->memory_offset;
}


// Component
int microECSComponentRegister(int size)
{
  assert(components_types_count < MAX_COMPONENTS);
  int id = components_types_count;
  components_types_count++;

  components_types_desc[id].component_size = size;
  if (id == 0)
    components_types_desc[id].memory_offset = 0;
  else
    components_types_desc[id].memory_offset = components_types_desc[id-1].memory_offset + components_types_desc[id-1].component_size;

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

  entity_data = NULL;
  is_entity_data_allocated = 0;
}

void microECSAllocateComponents()
{
  //compute entity size
  entity_size = 0;
  for (int i = 0; i < components_types_count; i++) {
    components_types_desc[i].memory_offset = entity_size;
    entity_size += components_types_desc[i].component_size;
  }

  //allocate entity data
  if (is_entity_data_allocated == 0) {
    entity_data = malloc(entity_size * MAX_ENTITIES);
    is_entity_data_allocated = 1;
  }

  printf("ECS allocated %lu kilobytes for entities\n", entity_size * MAX_ENTITIES / 1024);
}

void microECSFreeComponents()
{
  if (is_entity_data_allocated == 1) {
    free(entity_data);
    is_entity_data_allocated = 0;
    entity_data = NULL;
  }
}

void microECSRun(float dt)
{
  for (int i = 0; i < systems_count; i++) {
    if (systems[i].system != NULL) {
      systems[i].system(dt);
    }
  }
}

ecs_component_query microECSQueryComponent(int componentTypeId)
{
  ecs_component_query query;
  query.entityIds = &query_buf[0];
  query.size = 0;
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 1 && entities[i].components[componentTypeId] == 1) {
      query.entityIds[query.size] = i;
      query.size++;
    }
  }
  return query;
}

ecs_component_query microECSQueryByComponents(int* componentTypeIds, int size)
{
  ecs_component_query query;
  query.entityIds = &query_buf[0];
  query.size = 0;
  for (int i = 0; i < entities_count; i++) {
    if (entities[i].alive == 0) continue;
    int found = 1;
    for (int j = 0; j < size; j++) {
      if (entities[i].components[componentTypeIds[j]] != 1) {
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
