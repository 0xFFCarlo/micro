#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define GUARD_CONTENT 0xAB
#define VERBOSE 0

typedef struct MEM_INFO
{
  unsigned char guard;
  void *address;
  size_t size;
  struct MEM_INFO *next;
} MEM_INFO;

MEM_INFO *head = NULL;

void *malloc_debug(size_t size, char *file, int line)
{
  void *ptr = malloc(size + sizeof(MEM_INFO));
  if (ptr)
  {
    MEM_INFO *info = (MEM_INFO *)(ptr + size); // Shift MEM_INFO to the end of the allocated block
    info->address = ptr;
    info->size = size;
    info->guard = GUARD_CONTENT; // Set guard byte
    info->next = head;
    head = info;

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
  MEM_INFO **current = &head;
  while (*current)
  {
    if ((*current)->address == ptr)
    {
      MEM_INFO *info = *current;

      // Check the guard byte for corruption
      if (info->guard != GUARD_CONTENT)
      {
        if (VERBOSE) printf("Memory corruption detected at %p, file %s, line %d\n", ptr, file, line);
        return;
      }

      if (VERBOSE) printf("Freeing memory at %p, file %s, line %d\n", info->address, file, line);
      *current = (*current)->next;
      free(info->address); // Free at the original address
      return;
    }
    current = &(*current)->next;
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
    MEM_INFO **current = &head;
    while (*current)
    {
        if ((*current)->address == ptr)
        {
            MEM_INFO *info = *current;

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
        current = &(*current)->next;
    }

    if (VERBOSE) printf("Attempted to reallocate unallocated memory at %p, file %s, line %d\n", ptr, file, line);
    return NULL;
}

void memory_check_leaks()
{
  if (head)
  {
    printf("Memory leaks detected:\n");
    MEM_INFO *current = head;
    while (current)
    {
      // Check the guard byte for corruption
      if (current->guard != GUARD_CONTENT)
      {
        printf("Memory corruption detected at %p\n", current->address);
        return;
      }

      printf("Leaked %zu bytes at %p\n", current->size, current->address);
      current = current->next;
    }
  }
  else
  {
    printf("No memory leaks detected.\n");
  }
}

void memory_check_corruption()
{
  if (head)
  {
    MEM_INFO *current = head;
    while (current)
    {
      // Check the guard byte for corruption
      if (current->guard != GUARD_CONTENT)
        printf("Memory corruption detected at %p\n", current->address);

      current = current->next;
    }
  }
}
