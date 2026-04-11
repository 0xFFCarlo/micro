#include "ECS.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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

#ifndef MICRO_ECS_RUNTIME_CHECKS
#define MICRO_ECS_RUNTIME_CHECKS 1
#endif

////////////////////////////////////
////// DATA STRUCTURES /////////////
////////////////////////////////////
typedef struct
{
  uint8_t state;
  uint64_t components;
  uint16_t *component_offset;
  void *data;
  void (*free_entity)(int);
} entity_desc;
static entity_desc entities[MAX_ENTITIES];
static uint32_t entities_len = 0;
static int freed_entities[MAX_ENTITIES];
static int freed_entities_count = 0;
static int *entities_to_remove;

// Component
typedef struct
{
  int component_size;
  void (*freeComponent)(void *);
} component_desc;
static component_desc components_types_desc[MAX_COMPONENTS];
static unsigned int components_types_count = 0;
static void *components[MAX_COMPONENTS];
// NOTE: this bounds the number of possibel entities to 2^16 - 1
static uint16_t components_count[MAX_COMPONENTS];
static uint16_t components_entity_ref[MAX_COMPONENTS][MAX_ENTITIES];
static bool is_components_data_allocated = false;
static MicroECSNotifyFreedEntitiesCb notify_freed_entities_cb = NULL;

// System
static MicroECSSystem systems[MAX_SYSTEMS];
static unsigned int systems_count = 0;

#if MICRO_ECS_RUNTIME_CHECKS
#define ECS_VALID_ENTITY(entityId) \
  ((entityId) >= 0 && (entityId) < (int)entities_len)
#define ECS_VALID_COMPONENT(componentTypeId) \
  ((componentTypeId) >= 0 && \
   (componentTypeId) < (int)components_types_count)
#define ECS_VALID_SYSTEM(systemId) \
  ((systemId) >= 0 && (systemId) < (int)systems_count)
#define ECS_RUNTIME_ASSERT(cond) assert(cond)
#else
#define ECS_VALID_ENTITY(entityId) (true)
#define ECS_VALID_COMPONENT(componentTypeId) (true)
#define ECS_VALID_SYSTEM(systemId) (true)
#define ECS_RUNTIME_ASSERT(cond) ((void)0)
#endif

#define ECS_GUARD_ENTITY(entityId)                                               \
  do                                                                             \
  {                                                                              \
    if (!ECS_VALID_ENTITY(entityId))                                             \
      return;                                                                    \
  }                                                                              \
  while (0)

#define ECS_GUARD_ENTITY_RET(entityId, ret)                                      \
  do                                                                             \
  {                                                                              \
    if (!ECS_VALID_ENTITY(entityId))                                             \
      return (ret);                                                              \
  }                                                                              \
  while (0)

#define ECS_GUARD_COMPONENT(componentTypeId)                                     \
  do                                                                             \
  {                                                                              \
    if (!ECS_VALID_COMPONENT(componentTypeId))                                   \
      return;                                                                    \
  }                                                                              \
  while (0)

#define ECS_GUARD_COMPONENT_RET(componentTypeId, ret)                            \
  do                                                                             \
  {                                                                              \
    if (!ECS_VALID_COMPONENT(componentTypeId))                                   \
      return (ret);                                                              \
  }                                                                              \
  while (0)

#define ECS_GUARD_SYSTEM(systemId)                                               \
  do                                                                             \
  {                                                                              \
    if (!ECS_VALID_SYSTEM(systemId))                                             \
      return;                                                                    \
  }                                                                              \
  while (0)

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
    assert(entities_len < MAX_ENTITIES);
    id = entities_len;
    entities_len++;
  }

  SETBITS(entities[id].state, ENTITY_ALIVE_MASK);
  CLEARBITS(entities[id].state, ENTITY_FREED_MASK);
  entities[id].free_entity = free;
  entities[id].data = data;
  entities[id].components = 0; // No components enabled
  entities[id].component_offset = calloc(components_types_count,
                                         sizeof(uint16_t));
  assert(entities[id].component_offset != NULL || components_types_count == 0);

  return id;
}

void microECSEntityQueueFree(int entityId)
{
  ECS_GUARD_ENTITY(entityId);

  // Prevent queuing twice
  if ((entities[entityId].state & ENTITY_ALIVE_MASK) == 0)
    return;
  CLEARBITS(entities[entityId].state, ENTITY_ALIVE_MASK);
  vec_append(entities_to_remove, &entityId);
}

void microECSEntityFree(int entityId)
{
  ECS_GUARD_ENTITY(entityId);

  if (entities[entityId].state & ENTITY_FREED_MASK)
    return;

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
  if (entities[entityId].component_offset != NULL)
    free(entities[entityId].component_offset);
  SETBITS(entities[entityId].state, ENTITY_FREED_MASK);

  // Add to freed entities
  assert(freed_entities_count < MAX_ENTITIES);
  freed_entities[freed_entities_count] = entityId;
  freed_entities_count++;
}

int microECSEntityIsAlive(int entityId)
{
  ECS_GUARD_ENTITY_RET(entityId, 0);
  return (entities[entityId].state & ENTITY_ALIVE_MASK) != 0;
}

void *microECSEntityGetData(int entityId)
{
  ECS_GUARD_ENTITY_RET(entityId, NULL);
  return entities[entityId].data;
}

void microECSEntitySetData(int entityId, void *data)
{
  ECS_GUARD_ENTITY(entityId);
  entities[entityId].data = data;
}

void microECSEntitySetFreeData(int entityId, void (*free)(int))
{
  ECS_GUARD_ENTITY(entityId);
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
  ECS_RUNTIME_ASSERT(ECS_VALID_ENTITY(entityId));
  ECS_RUNTIME_ASSERT(ECS_VALID_COMPONENT(componentTypeId));
  ECS_RUNTIME_ASSERT(data != NULL);

  // Does entity have component already?
  assert(microECSEntityHasComponent(entityId, componentTypeId) == 0);

  // Get component description
  component_desc *cdesc = &components_types_desc[componentTypeId];

  // Update entity flags and offsets
  entities[entityId].components |= (1ULL << componentTypeId);
  entities[entityId]
    .component_offset[componentTypeId] = components_count[componentTypeId];
  assert(components_count[componentTypeId] < MAX_ENTITIES);

  // Store component and update entity reference
  components_count[componentTypeId]++;
  void *component = microECSEntityIndexComponent(entityId, componentTypeId);
  memcpy(component, data, cdesc->component_size);
  components_entity_ref[componentTypeId]
                       [components_count[componentTypeId] - 1] = entityId;
}

void microECSEntityRemoveComponent(int entityId, const int componentTypeId)
{
  ECS_GUARD_ENTITY(entityId);
  ECS_GUARD_COMPONENT(componentTypeId);
  if (!microECSEntityHasComponent(entityId, componentTypeId))
    return;

  // Upadte entity flags
  entities[entityId].components &= ~(1ULL << componentTypeId);

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
}

void *microECSEntityGetComponent(int entityId, const int componentTypeId)
{
  ECS_GUARD_ENTITY_RET(entityId, NULL);
  ECS_GUARD_COMPONENT_RET(componentTypeId, NULL);
  if (microECSEntityHasComponent(entityId, componentTypeId) == 0)
    return NULL;
#ifdef DEBUG_MODE
  if ((entities[entityId].state & ENTITY_FREED_MASK) != 0 ||
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
  ECS_GUARD_ENTITY_RET(entityId, 0);
  ECS_GUARD_COMPONENT_RET(componentTypeId, 0);
  return (entities[entityId].components & (1ULL << componentTypeId)) != 0;
}

void microECSEntityFreeAll()
{
  for (unsigned int i = 0; i < entities_len; i++)
    microECSEntityFree(i);
}

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
  ECS_GUARD_COMPONENT_RET(componentTypeId, NULL);
  return components[componentTypeId];
}

int microECSComponentsCount(int componentTypeId)
{
  ECS_GUARD_COMPONENT_RET(componentTypeId, 0);
  return components_count[componentTypeId];
}

int microECSComponentGetEntityId(int componentTypeId, int index)
{
  ECS_GUARD_COMPONENT_RET(componentTypeId, -1);
  if (index < 0 || index >= components_count[componentTypeId])
    return -1;
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
  ECS_GUARD_SYSTEM(systemId);
  if (systems[systemId].system_free != NULL)
    systems[systemId].system_free();
  memset(&systems[systemId], 0, sizeof(MicroECSSystem));
}

//////////////////////////////////////////
////// ENTITY SYSTEM IMPLEMENTATION //////
//////////////////////////////////////////
int microECSInit()
{
  // Initialize entity system
  for (int i = 0; i < MAX_ENTITIES; i++)
    freed_entities[i] = -1;
  entities_to_remove = vec_new(sizeof(int));
  return 0;
}

void microECSAllocateComponents()
{
  assert(is_components_data_allocated == false);

  // allocate components vectors
  unsigned int memory_used = 0;
  for (unsigned int i = 0; i < components_types_count; i++)
  {
    const unsigned int component_size = components_types_desc[i].component_size;
    components[i] = malloc(component_size * MAX_ENTITIES);
    components_count[i] = 0;
    memory_used += component_size * MAX_ENTITIES;
  }
  is_components_data_allocated = true;

  unsigned long memory = memory_used;
  memory += sizeof(components_count);
  memory += sizeof(components_entity_ref);
  memory += sizeof(entities);
  memory += sizeof(freed_entities);
  memory += sizeof(systems);
  memory += sizeof(components_types_desc);
  memory += sizeof(freed_entities_count);
  memory += sizeof(entities_len);
  memory += sizeof(systems_count);
  memory += sizeof(components_types_count);
  memory += sizeof(is_components_data_allocated);

  debug_print("microECS allocated %d kb for %d components\n",
              (int)(memory / (double)1024), components_types_count);
}

void microECSFree()
{
  // Remove entities that were marked for removal
  while (vec_len(entities_to_remove) > 0)
  {
    const int eid = *(int *)vec_back(entities_to_remove);
    vec_pop_back(entities_to_remove);
    microECSEntityFree(eid);
  }

  // Free entities
  microECSEntityFreeAll();
  if (entities_to_remove != NULL)
    vec_free(entities_to_remove);

  // Free components
  if (is_components_data_allocated)
  {
    // Free components vectors
    for (unsigned int i = 0; i < components_types_count; i++)
      free(components[i]);

    is_components_data_allocated = false;
  }

  // Free systems
  for (unsigned int i = 0; i < systems_count; i++)
    if (systems[i].system_free != NULL)
      systems[i].system_free();
}

void microECSRun(float dt)
{
  // Notify of queue to be freed entities
  if (notify_freed_entities_cb != NULL && vec_len(entities_to_remove) > 0)
    notify_freed_entities_cb((int *)entities_to_remove,
                             vec_len(entities_to_remove));

  // Remove entities that were marked for removal, fifo
  for (uint32_t i = 0; i < vec_len(entities_to_remove); i++)
    microECSEntityFree(entities_to_remove[i]);
  vec_clear(entities_to_remove);

  // Update systems
  for (unsigned int i = 0; i < systems_count; i++)
    if (systems[i].update != NULL)
      systems[i].update(dt);
}

void microECSSetNotifyFreedEntitiesCb(MicroECSNotifyFreedEntitiesCb cb)
{
  notify_freed_entities_cb = cb;
}
