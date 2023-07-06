#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASHMAP_SIZE 2048
#define HASH_SEED 0xdeadbeef
#define GUARD_CONTENT 0xAB
#define VERBOSE 0

typedef struct MEM_INFO
{
  unsigned char guard;
  void *address;
  size_t size;
  struct MEM_INFO *next;
} MEM_INFO;

MEM_INFO* hashmap[HASHMAP_SIZE] = {NULL};

// MurmurHash64A
uint32_t hash_fun (uint64_t key) {
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = HASH_SEED ^ (8 * m);

    const uint64_t* data = &key;
    const uint64_t* end = data + 1;

    while(data != end) {
        uint64_t k = *data++;

        k *= m; 
        k ^= k >> r; 
        k *= m; 
        
        h ^= k;
        h *= m; 
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h % HASHMAP_SIZE;
}

void hashmap_add(void* ptr, MEM_INFO* info) {
  int index = hash_fun((uint64_t)ptr);
  info->next = hashmap[index];
  // if (hashmap[index] != NULL)
  //   printf("COLLISION\n");
  hashmap[index] = info;
}

MEM_INFO* hashmap_get(void* ptr) {
  int index = hash_fun((uint64_t)ptr);
  MEM_INFO* current = hashmap[index];
  while (current != NULL) {
    if (current->address == ptr)
      return current;
    current = current->next;
  }
  return NULL;
}

void hashmap_remove(void* ptr) {
  int index = hash_fun((uint64_t)ptr);
  MEM_INFO* current = hashmap[index];
  if (current == NULL) return;
  if (current->address == ptr) {
    hashmap[index] = current->next;
    return;
  }
  while (current->next != NULL) {
    if (current->next->address == ptr) {
      current->next = current->next->next;
      return;
    }
    current = current->next;
  }
}

void *malloc_debug(size_t size, char *file, int line)
{
  void *ptr = malloc(size + sizeof(MEM_INFO));
  if (ptr)
  {
    MEM_INFO *info = (MEM_INFO *)(ptr + size); // Shift MEM_INFO to the end of the allocated block
    info->address = ptr;
    info->size = size;
    info->guard = GUARD_CONTENT; // Set guard byte
    hashmap_add(ptr, info);

    if (VERBOSE) printf("Allocated %zu bytes at %p, file %s, line %d\n", size, info->address, file, line);
    return info->address;
  }
  else
  {
    return NULL;
  }
}

void free_debug(void *ptr, char *file, int line)
{
  MEM_INFO* info = hashmap_get(ptr);

  if (info != NULL) {
    // Check the guard byte for corruption
    if (info->guard != GUARD_CONTENT) {
      if (VERBOSE) printf("Memory corruption detected at %p, file %s, line %d\n", ptr, file, line);
      return;
    }

    if (VERBOSE) printf("Freeing memory at %p, file %s, line %d\n", info->address, file, line);
    hashmap_remove(ptr);
    free(info->address); // Free at the original address
    return;
  }

  if (VERBOSE) printf("Attempted to free unallocated memory at %p, file %s, line %d\n", ptr, file, line);
}

void *realloc_debug(void *ptr, size_t size, char *file, int line)
{
  if (ptr == NULL)
  {
    return malloc_debug(size, file, line);
  }

  if (size == 0)
  {
    free_debug(ptr, file, line);
    return NULL;
  }

  // Locate the existing memory block
  MEM_INFO* info = hashmap_get(ptr);
  if (info != NULL) {
    // Check the guard byte for corruption
    if (info->guard != GUARD_CONTENT)
    {
      if (VERBOSE) printf("Memory corruption detected at %p, file %s, line %d\n", ptr, file, line);
      return NULL;
    }

    // Allocate new block
    void *new_ptr = malloc_debug(size, file, line);
    if (new_ptr)
    {
      // Copy old data to new block
      memcpy(new_ptr, ptr, info->size < size ? info->size : size);

      // Free old block
      free_debug(ptr, file, line);
    }

    return new_ptr;
  }

  if (VERBOSE) printf("Attempted to reallocate unallocated memory at %p, file %s, line %d\n", ptr, file, line);
  return NULL;
}

void memory_check_leaks()
{
  int leaks = 0;
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    MEM_INFO *current = hashmap[i];
    while (current)
    {
      // Check the guard byte for corruption
      if (current->guard != GUARD_CONTENT)
      {
        printf("Memory corruption detected at %p\n", current->address);
        return;
      }

      printf("Leaked %zu bytes at %p\n", current->size, current->address);
      leaks++;
      current = current->next;
    }
  }
  
  if (leaks == 0)
    printf("No memory leaks detected.\n");
}

void memory_check_corruption()
{
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    MEM_INFO *current = hashmap[i];
    while (current)
    {
      // Check the guard byte for corruption
      if (current->guard != GUARD_CONTENT)
      {
        printf("Memory corruption detected at %p\n", current->address);
        return;
      }
      current = current->next;
    }
  }
  printf("No memory corruption detected.\n");
}
