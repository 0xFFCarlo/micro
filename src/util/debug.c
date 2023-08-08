#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configurations
#define HASHMAP_SIZE 2048
#define HASH_SEED 0xdeadbeef
#define GUARD_CONTENT 0xAB
#define GUARD_LENGTH 4
#define FILENAME_MAX_LENGTH 64

// Settings
#define VERBOSE 0
#define USE_BACKTRACE

// Colors
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_LIGHT_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_LIGHT_GRAY "\033[0;37m"
#define COLOR_DEFAULT "\033[0m"

#ifdef USE_BACKTRACE
#include <execinfo.h>
#endif

typedef struct LOCATION_INFO
{
  char filename[FILENAME_MAX_LENGTH];
  int line;
  struct LOCATION_INFO *next;
} LOCATION_INFO;

LOCATION_INFO *location_hashmap[HASHMAP_SIZE] = {NULL};
uint16_t location_ids = 0;

typedef struct MEM_INFO
{
  unsigned char guard[GUARD_LENGTH];
  void *address;
  size_t size;
  LOCATION_INFO *symbol;
  struct MEM_INFO *next;
} MEM_INFO;

MEM_INFO *allocations_hashmap[HASHMAP_SIZE] = {NULL};

#ifdef USE_BACKTRACE

void print_trace()
{
  void *array[5];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 5);
  strings = backtrace_symbols(array, size);

  for (i = 0; i < size; i++)
    printf("%s\n", strings[i]);

  free(strings);
}

char *backtrace_get()
{
  void *buffer[5];
  char **symbols;
  int nptrs;

  nptrs = backtrace(buffer, 5);
  symbols = backtrace_symbols(buffer, nptrs);
  if (symbols == NULL)
  {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  // get function names
  char **fun_names = malloc(sizeof(char *) * nptrs);
  for (int j = 0; j < nptrs; j++)
  {
    char *line = strdup(symbols[j]);
    char *token = strtok(line, " "); // Get the first token
    token = strtok(NULL, " ");       // Get the second token (library name)
    token = strtok(NULL, " ");       // Get the third token (the address)
    token = strtok(NULL, " ");       // Get the fourth token (the function name)

    fun_names[j] = NULL;
    if (token == NULL)
      continue;

    fun_names[j] = strdup(token);
    free(line);
  }

  // compute description size
  int description_size = 3; // '[' + ']' + '\0'
  for (int j = 0; j < nptrs; j++)
  {
    if (fun_names[j] == NULL)
      continue;
    description_size += strlen(fun_names[j]) + 3;
  }

  // build description
  char *call_description = malloc(sizeof(char) * description_size);
  call_description[0] = '[';
  call_description[1] = '\0';
  int skip_first = 2;
  for (int j = 0; j < nptrs; j++)
  {
    if (skip_first > 0)
    {
      skip_first--;
      if (fun_names[j] != NULL)
        free(fun_names[j]);
      continue;
    }
    if (fun_names[j] == NULL)
      continue;
    strcat(call_description, fun_names[j]);
    if (j != nptrs - 1)
      strcat(call_description, " < ");
    free(fun_names[j]);
  }
  strcat(call_description, "]\0");

  free(fun_names);
  free(symbols);

  return call_description;
}

#endif

// MurmurHash64A
uint32_t hash_fun(uint64_t key)
{
  const uint64_t m = 0xc6a4a7935bd1e995;
  const int r = 47;

  uint64_t h = HASH_SEED ^ (8 * m);

  const uint64_t *data = &key;
  const uint64_t *end = data + 1;

  while (data != end)
  {
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

uint32_t loc_hash_fun(const char *filename, int line)
{
  uint64_t key = 1;
  const int filename_length = strlen(filename);
  for (int i = 0; i < filename_length; i++)
    key *= filename[i];
  key += line;
  return hash_fun(key);
}

void alloc_hashmap_add(void *ptr, MEM_INFO *info)
{
  int index = hash_fun((uint64_t)ptr);
  info->next = allocations_hashmap[index];
  // if (allocations_hashmap[index] != NULL)
  //   printf("COLLISION\n");
  allocations_hashmap[index] = info;
}

MEM_INFO *alloc_hashmap_get(void *ptr)
{
  int index = hash_fun((uint64_t)ptr);
  MEM_INFO *current = allocations_hashmap[index];
  while (current != NULL)
  {
    if (current->address == ptr)
      return current;
    current = current->next;
  }
  return NULL;
}

void alloc_hashmap_remove(void *ptr)
{
  int index = hash_fun((uint64_t)ptr);
  MEM_INFO *current = allocations_hashmap[index];
  if (current == NULL)
    return;
  if (current->address == ptr)
  {
    allocations_hashmap[index] = current->next;
    return;
  }
  while (current->next != NULL)
  {
    if (current->next->address == ptr)
    {
      current->next = current->next->next;
      return;
    }
    current = current->next;
  }
}

LOCATION_INFO *loc_hashmap_get(const char *filename, int line)
{
  int index = loc_hash_fun(filename, line);
  LOCATION_INFO *current = location_hashmap[index];
  while (current != NULL)
  {
    if (strcmp(current->filename, filename) == 0 && current->line == line)
      return current;
    current = current->next;
  }
  return NULL;
}

LOCATION_INFO *loc_hashmap_add(const char *filename, int line)
{
  LOCATION_INFO *location = loc_hashmap_get(filename, line);
  if (location != NULL)
    return location;

  int index = loc_hash_fun(filename, line);
  LOCATION_INFO *new_location = malloc(sizeof(LOCATION_INFO));
  int filename_length = strlen(filename);
  if (filename_length > FILENAME_MAX_LENGTH)
    filename_length = FILENAME_MAX_LENGTH;
  strncpy(new_location->filename, filename, filename_length);
  new_location->line = line;
  new_location->next = location_hashmap[index];
  location_hashmap[index] = new_location;
  return new_location;
}

int is_guard_valid(MEM_INFO *info)
{
  for (int i = 0; i < GUARD_LENGTH; i++)
    if (info->guard[i] != GUARD_CONTENT)
      return 0;
  return 1;
}

void *malloc_debug(const size_t size, char *file, const int line)
{
  void *ptr = malloc(size + sizeof(MEM_INFO));
  if (ptr)
  {
    MEM_INFO *info = (MEM_INFO *)(ptr + size); // Shift MEM_INFO to the end of
                                               // the allocated block
    info->address = ptr;
    info->size = size;
    memset(info->guard, GUARD_CONTENT, GUARD_LENGTH); // Set guard bytes

    // store location info
    char *last_slash = strrchr(file, '/');
    if (last_slash == NULL)
      last_slash = file;
    else
      last_slash++;
    char *loc_name = last_slash;

#ifdef USE_BACKTRACE
    char *call_description = backtrace_get();
    loc_name = call_description;
    loc_name = malloc(strlen(call_description) + strlen(last_slash) + 2);
    strcpy(loc_name, last_slash);
    strcat(loc_name, " ");
    strcat(loc_name, call_description);
    strcat(loc_name, "\0");
    free(call_description);
#endif

    info->symbol = loc_hashmap_add(loc_name, line);

#ifdef USE_BACKTRACE
    free(loc_name);
#endif

    alloc_hashmap_add(ptr, info);

    if (VERBOSE)
      printf("Allocated %zu bytes at %p, file %s, line %d\n", size,
             info->address, file, line);
    return info->address;
  }
  else
  {
    return NULL;
  }
}

void free_debug(void *ptr, const char *file, const int line)
{
  MEM_INFO *info = alloc_hashmap_get(ptr);

  if (info != NULL)
  {
    // Check the guard byte for corruption
    if (is_guard_valid(info) == 0)
    {
      if (VERBOSE)
        printf("Memory corruption detected at %p, file %s, line %d\n", ptr,
               file, line);
      return;
    }

    if (VERBOSE)
      printf("Freeing memory at %p, file %s, line %d\n", info->address, file,
             line);
    alloc_hashmap_remove(ptr);
    free(info->address); // Free at the original address
    return;
  }

  if (VERBOSE)
    printf("Attempted to free unallocated memory at %p, file %s, line %d\n",
           ptr, file, line);
}

void *realloc_debug(void *ptr, const size_t size, char *file, const int line)
{
  if (ptr == NULL)
  {
    // MALLOC COPY (to reduce stack depth)
    void *address = malloc(size + sizeof(MEM_INFO));
    if (address)
    {
      MEM_INFO
      *info = (MEM_INFO *)(address + size); // Shift MEM_INFO to the end of
                                            // the allocated block
      info->address = address;
      info->size = size;
      memset(info->guard, GUARD_CONTENT, GUARD_LENGTH); // Set guard bytes

      // store location info
      char *last_slash = strrchr(file, '/');
      if (last_slash == NULL)
        last_slash = file;
      else
        last_slash++;
      char *loc_name = last_slash;

#ifdef USE_BACKTRACE
      char *call_description = backtrace_get();
      loc_name = call_description;
      loc_name = malloc(strlen(call_description) + strlen(last_slash) + 2);
      strcpy(loc_name, last_slash);
      strcat(loc_name, " ");
      strcat(loc_name, call_description);
      strcat(loc_name, "\0");
      free(call_description);
#endif

      info->symbol = loc_hashmap_add(loc_name, line);

#ifdef USE_BACKTRACE
      free(loc_name);
#endif

      alloc_hashmap_add(address, info);

      if (VERBOSE)
        printf("Allocated %zu bytes at %p, file %s, line %d\n", size,
               info->address, file, line);
      return info->address;
    }
    else
    {
      return NULL;
    }
    ////////////////////
  }

  if (size == 0)
  {
    free_debug(ptr, file, line);
    return NULL;
  }

  // Locate the existing memory block
  MEM_INFO *info = alloc_hashmap_get(ptr);
  if (info != NULL)
  {
    // Check the guard byte for corruption
    if (is_guard_valid(info) == 0)
    {
      if (VERBOSE)
        printf("Memory corruption detected at %p, file %s, line %d\n", ptr,
               file, line);
      return NULL;
    }

    // Allocate new block
    // MALLOC COPY (to reduce stack depth)
    void *new_ptr = NULL;
    void *address = malloc(size + sizeof(MEM_INFO));
    if (address)
    {
      MEM_INFO
      *info = (MEM_INFO *)(address + size); // Shift MEM_INFO to the end of
                                            // the allocated block
      info->address = address;
      info->size = size;
      memset(info->guard, GUARD_CONTENT, GUARD_LENGTH); // Set guard bytes

      // store location info
      char *last_slash = strrchr(file, '/');
      if (last_slash == NULL)
        last_slash = file;
      else
        last_slash++;
      char *loc_name = last_slash;

#ifdef USE_BACKTRACE
      char *call_description = backtrace_get();
      loc_name = call_description;
      loc_name = malloc(strlen(call_description) + strlen(last_slash) + 2);
      strcpy(loc_name, last_slash);
      strcat(loc_name, " ");
      strcat(loc_name, call_description);
      strcat(loc_name, "\0");
      free(call_description);
#endif

      info->symbol = loc_hashmap_add(loc_name, line);

#ifdef USE_BACKTRACE
      free(loc_name);
#endif

      alloc_hashmap_add(address, info);

      if (VERBOSE)
        printf("Allocated %zu bytes at %p, file %s, line %d\n", size,
               info->address, file, line);
      new_ptr = info->address;
    }
    else
    {
      new_ptr = NULL;
    }
    ////////////////////

    if (new_ptr)
    {
      // Copy old data to new block
      memcpy(new_ptr, ptr, info->size < size ? info->size : size);

      // Free old block
      free_debug(ptr, file, line);
    }

    return new_ptr;
  }

  if (VERBOSE)
    printf("Attempted to reallocate unallocated memory at %p, file %s, line "
           "%d\n",
           ptr, file, line);
  return NULL;
}

void memory_check_leaks()
{
  printf("Checking for memory leaks...\n");
  int leaks = 0;
  for (int i = 0; i < HASHMAP_SIZE; i++)
  {
    MEM_INFO *current = allocations_hashmap[i];
    // printf(" current %p\n", current);
    // printf("-> Checking hashmap bucket %d/%d\n", i + 1, HASHMAP_SIZE);
    while (current)
    {
      // Check the guard byte for corruption
      if (is_guard_valid(current) == 0)
      {
        printf("-> Memory corruption detected at %14p, from %8s line %d\n",
               current->address, current->symbol->filename,
               current->symbol->line);
        return;
      }

      printf("-> Leaked %s%10lu%s bytes at %14p, from %s%8s%s line %d\n",
             COLOR_YELLOW, current->size, COLOR_DEFAULT, current->address,
             COLOR_LIGHT_BLUE, current->symbol->filename, COLOR_DEFAULT,
             current->symbol->line);
      leaks++;
      current = current->next;
    }
  }

  if (leaks == 0)
  {
    printf("%sNo memory leaks detected.%s\n", COLOR_GREEN, COLOR_DEFAULT);
    printf("%sNo memory corruption detected.%s\n", COLOR_GREEN, COLOR_DEFAULT);
  }
  printf("Done\n");
}

void memory_check_corruption()
{
  for (int i = 0; i < HASHMAP_SIZE; i++)
  {
    MEM_INFO *current = allocations_hashmap[i];
    while (current)
    {
      // Check the guard byte for corruption
      if (is_guard_valid(current) == 0)
      {
        printf("-> Memory corruption detected at %14p, from %8s line %d\n",
               current->address, current->symbol->filename,
               current->symbol->line);
        return;
      }
      current = current->next;
    }
  }
  // printf("No memory corruption detected.\n");
}

void assert_debug(int condition, char *condition_str, char *file, int line)
{
  if (!condition)
  {
    printf("%sAssertion %s%s%s failed at %s%s line %d%s\n", COLOR_RED,
           COLOR_GREEN, condition_str, COLOR_RED, COLOR_MAGENTA, file, line,
           COLOR_DEFAULT);

    // Print stack trace
#ifdef USE_BACKTRACE
    char *call_description = backtrace_get();
    printf("%sCall stack: %s%s\n", COLOR_YELLOW, call_description,
           COLOR_DEFAULT);
    free(call_description);
#endif

    exit(1);
  }
}

uint32_t print_id = 0;

void _print_debug(const char *file, const int line, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  printf("%s--> %-7d %-16s line %-5d   ", COLOR_CYAN, print_id, file, line);
  vprintf(format, args);
  printf("%s\n", COLOR_DEFAULT);
  va_end(args);
}
