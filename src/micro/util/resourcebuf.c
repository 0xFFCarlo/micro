// #include "resourcebuf.h"
// #include "vector.h"
// #include <stdio.h>
// #include <string.h>
//
// typedef struct MicroResource
// {
//   uint64_t hash;
//   int id;
// } MicroResource;
//
// typedef struct MicroResourceBuf
// {
//   MicroResource *hashmap;
//   uint32_t hashmap_capacity;
//   uint32_t hashmap_count;
//   Vector removed_ids;
//   Vector data;
// } MicroResourceBuf;
//
// static uint64_t fnv1a_hash(const char *str)
// {
//   const uint64_t fnv_prime = 0x100000001b3;
//   uint64_t hash = 0xcbf29ce484222325; // FNV offset basis
//
//   while (*str)
//   {
//     hash ^= (uint64_t)(*str++);
//     hash *= fnv_prime;
//   }
//
//   if (hash == 0)
//     hash = 1;
//
//   return hash;
// }
//
// MicroResourceBuf* microResourceBufNew(int resourceSize, int iniCapacity)
// {
//   MicroResourceBuf* rbuf = malloc(sizeof(MicroResourceBuf));
//   rbuf->hashmap = malloc(sizeof(MicroResource) * iniCapacity * 2);
//   rbuf->hashmap_capacity = iniCapacity * 2;
//   rbuf->hashmap_count = 0;
//   rbuf->removed_ids = vector_create_with_capacity(sizeof(int), iniCapacity);
//   rbuf->data = vector_create_with_capacity(resourceSize, iniCapacity);
//   return rbuf;
// }
//
// int microResourceBufStore(MicroResourceBuf* rbuf, const char *name, void* data)
// {
//   uint64_t hash = fnv1a_hash(name) % rbuf->hashes_capacity;
//   int hashmap_idx = -1;
//   int resource_idx = -1;
//
//   if (rbuf->removed_ids.size) 
//   {
//     resource_idx = *(int*)vector_last(&rbuf->removed_ids);
//     vector_pop_back(&rbuf->removed_ids);
//     void* old_data = vector_at(&rbuf->data, resource_idx);
//     memcpy(old_data, data, rbuf->data.data_size);
//   }
//   else
//   {
//     resource_idx = rbuf->data.size;
//     vector_push_back(&rbuf->data, data);
//   }
//
//   for (int i = 0; i < (int)rbuf->hashes_capacity; i++)
//   {
//     int idx = (hash + i) % rbuf->hashes_capacity;
//     if (rbuf->hashes[idx] != 0)
//       continue;
//     spot = idx;
//     break;
//   }
//
//   if (spot == -1)
//   {
//     printf("Error: no spot found for resource %s\n", name);
//     return -1;
//   }
//
//   resources_map[type][spot].hash = hash;
//   resources_map[type][spot].id = id;
//   resources_inv_map[type][id] = spot;
//
//   return spot;
// }
//
// int microResourceBufRemove(MicroResourceBuf* rbuf, int id)
// {
//
// }
//
// int microResourceBufGetId(MicroResourceBuf* rbuf, const char *name)
// {
//
// }
//
// void* microResourceBufGetData(MicroResourceBuf* rbuf, int id)
// {
//
// }
