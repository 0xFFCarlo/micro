#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

HashMap hashmap_create(size_t key_size, size_t map_size,
                       uint32_t (*hash_fun)(const void *key),
                       void (*free_fun)(void *value))
{
  HashMap map = {0};
  map.data = malloc(sizeof(void *) * map_size);
  memset(map.data, 0, sizeof(void *) * map_size);
  map.size = map_size;
  map.key_size = key_size;
  map.hash_fun = hash_fun;
  map.free_fun = free_fun;
  map.entries_count = 0;
  return map;
}

void hashmap_free_all_nodes(void *node, size_t key_size, void (*free_fun)(void *value))
{
  if (!node)
    return;

  void *key = node;
  void *data = key + key_size;
  void **next_ptr = data + sizeof(void *);
  void **value_ptr = (void **)data;
  if (free_fun)
    free_fun(*value_ptr);
  hashmap_free_all_nodes(*next_ptr, key_size, free_fun);
  free(node);
}

void hashmap_free(HashMap *map)
{
  for (size_t i = 0; i < map->size; i++)
    if (map->data[i])
      hashmap_free_all_nodes(map->data[i], map->key_size, map->free_fun);
  free(map->data);
  map->data = NULL;
  map->key_size = 0;
  map->entries_count = 0;
}

void* hashmap_get_next(HashMap *map, void *entry)
{
  void *data = entry + map->key_size;
  void *next = data + sizeof(void *);
  return *(void **)next;
}

void hashmap_insert(HashMap *map, const void *key, void *value)
{
  uint32_t hash = map->hash_fun(key) % map->size;
  void *node = malloc(map->key_size + sizeof(void *) * 2);
  void *data = node + map->key_size;
  void *next = data + sizeof(void *);
  memcpy(node, key, map->key_size);
  memcpy(data, &value, sizeof(void *));
  memcpy(next, &map->data[hash], sizeof(void *));
  map->data[hash] = node;
  map->entries_count++;
}

void *hashmap_get(HashMap *map, const void *key)
{
  const uint32_t hash = map->hash_fun(key) % map->size;
  void *node = map->data[hash];
  while (node)
  {
    void *data = node + map->key_size;
    void *next = data + sizeof(void *);
    if (memcmp(node, key, map->key_size) == 0)
      return *(void **)data;
    node = *(void **)next;
  }
  return NULL;
}

void hashmap_remove(HashMap *map, const void *key)
{
  uint32_t hash = map->hash_fun(key) % map->size;
  void *node = map->data[hash];
  void *prev = NULL;
  while (node)
  {
    void *data = node + map->key_size;
    void *next = data + sizeof(void *);
    if (memcmp(node, key, map->key_size) != 0)
    {
      prev = node;
      node = *(void **)next;
      continue;
    }

    if (prev)
    {
      void *prev_next = prev + map->key_size + sizeof(void *);
      memcpy(prev_next, next, sizeof(void *));
    }
    else
    {
      map->data[hash] = *(void **)next;
    }
    free(node);
    map->entries_count--;
    return;
  }
}

void hashmap_clear(HashMap *map)
{
  for (size_t i = 0; i < map->size; i++)
    if (map->data[i])
      hashmap_free_all_nodes(map->data[i], map->key_size, map->free_fun);
  memset(map->data, 0, sizeof(void *) * map->size);
  map->entries_count = 0;
}
