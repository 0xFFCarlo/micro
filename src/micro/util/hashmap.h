#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>

typedef struct HashMap
{
  void **data;
  size_t size;
  size_t key_size;
  size_t entries_count;
  uint32_t (*hash_fun)(const void *key);
  void (*free_fun)(void *value);
} HashMap;

HashMap hashmap_create(size_t key_size, size_t map_size,
                       uint32_t (*hash_fun)(const void *key),
                       void (*free_fun)(void *value));
void hashmap_free(HashMap *map);
void* hashmap_get_next(HashMap *map, void *entry);
void hashmap_insert(HashMap *map, const void *key, void *value);
void *hashmap_get(HashMap *map, const void *key);
void hashmap_remove(HashMap *map, const void *key);
void hashmap_clear(HashMap *map);

#endif /* end of include guard: HASHMAP_H */
