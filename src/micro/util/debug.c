#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configurations
#define HASHMAP_SIZE 2048
#define HASH_SEED 0xdeadbeef
#define GUARD_CONTENT 0xAB
#define GUARD_LENGTH 4
#define FILENAME_MAX_LENGTH 64
#define MULTITHREAD // Comment this line to disable multithreading
#define BACKTRACE_LOG_MAX_LEN 1024
// locks

// Settings
#define VERBOSE 0
#define USE_BACKTRACE

// Colors
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_LIGHT_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_LIGHT_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_LIGHT_CYAN "\033[1:36m"
#define COLOR_LIGHT_GRAY "\033[0;37m"
#define COLOR_DEFAULT "\033[0m"

#ifdef USE_BACKTRACE
#include <execinfo.h>
#endif

#ifdef MULTITHREAD
#include <pthread.h>
#endif

typedef struct LOCATION_INFO
{
  char filename[FILENAME_MAX_LENGTH];
  int line;
  struct LOCATION_INFO *next;
} LOCATION_INFO;

static LOCATION_INFO *location_hashmap[HASHMAP_SIZE] = {NULL};

typedef struct MEM_INFO
{
  unsigned char guard[GUARD_LENGTH];
  void *address;
  size_t size;
  LOCATION_INFO *symbol;
  struct MEM_INFO *next;
} MEM_INFO;

static MEM_INFO *allocations_hashmap[HASHMAP_SIZE] = {NULL};

static uint64_t memory_allocated = 0;
static char backtrace_log[BACKTRACE_LOG_MAX_LEN];

#ifdef MULTITHREAD
static pthread_mutex_t memory_mutex;
static uint8_t memory_mutex_initialized = 0;
#endif

#ifdef USE_BACKTRACE

void backtrace_get(char *buffer, uint32_t buffer_len, int skip_n_lines)
{
  void *bt_buffer[10];
  char **symbols;
  int nptrs;
  char *buf_ptr = buffer;
  char *buf_end = buffer + buffer_len;
  int ret;

  nptrs = backtrace(bt_buffer, 10);
  symbols = backtrace_symbols(bt_buffer, nptrs);
  if (symbols == NULL)
  {
    perror("backtrace_symbols");
    return;
  }

  for (int j = skip_n_lines; j < nptrs; j++)
  {
    ret = sprintf(buf_ptr, "%s\n", symbols[j]);
    if (ret < 0 || buf_ptr + ret >= buf_end)
      break;

    buf_ptr += ret;
  }
  free(symbols);
}

void print_trace()
{
  backtrace_get(backtrace_log, BACKTRACE_LOG_MAX_LEN, 2);
  printf("%s", COLOR_LIGHT_CYAN);
  printf("%s\n", backtrace_log);
  printf("%s", COLOR_DEFAULT);
}

#else

void print_trace()
{
  printf("Backtrace not supported on this platform.\n");
}

#endif

// MurmurHash64A
static uint32_t hash_fun(uint64_t key)
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

static uint32_t loc_hash_fun(const char *filename, int line)
{
  uint64_t key = 1;
  const int filename_length = strlen(filename);
  for (int i = 0; i < filename_length; i++)
    key *= filename[i];
  key += line;
  return hash_fun(key);
}

static void alloc_hashmap_add(void *ptr, MEM_INFO *info)
{
  int index = hash_fun((uint64_t)ptr);
  info->next = allocations_hashmap[index];
  // if (allocations_hashmap[index] != NULL)
  //   printf("COLLISION\n");
  allocations_hashmap[index] = info;
}

static MEM_INFO *alloc_hashmap_get(void *ptr)
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

static void alloc_hashmap_remove(void *ptr)
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

static LOCATION_INFO *loc_hashmap_get(const char *filename, int line)
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

static LOCATION_INFO *loc_hashmap_add(const char *filename, int line)
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

static int is_guard_valid(MEM_INFO *info)
{
  for (int i = 0; i < GUARD_LENGTH; i++)
    if (info->guard[i] != GUARD_CONTENT)
      return 0;
  return 1;
}

void *malloc_debug(const size_t size, char *file, const int line)
{
#ifdef MULTITHREAD
  if (!memory_mutex_initialized)
  {
    pthread_mutex_init(&memory_mutex, NULL);
    memory_mutex_initialized = 1;
  }
  pthread_mutex_lock(&memory_mutex);
#endif

  void *ptr = malloc(size + sizeof(MEM_INFO));
  memory_allocated += size;

  if (ptr)
  {
    MEM_INFO *info = (MEM_INFO *)(ptr + size); // Shift MEM_INFO to the end of
                                               // the allocated block
    info->next = NULL;
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

    info->symbol = loc_hashmap_add(loc_name, line);

    if (VERBOSE)
      printf("Allocated %zu bytes at %p, file %s, line %d\n", size,
             info->address, file, line);

    alloc_hashmap_add(ptr, info);

#ifdef MULTITHREAD
    pthread_mutex_unlock(&memory_mutex);
#endif
    return info->address;
  }
  else
  {
#ifdef MULTITHREAD
    pthread_mutex_unlock(&memory_mutex);
#endif
    return NULL;
  }
}

void free_debug(void *ptr, const char *file, const int line)
{
#ifdef MULTITHREAD
  if (!memory_mutex_initialized)
  {
    pthread_mutex_init(&memory_mutex, NULL);
    memory_mutex_initialized = 1;
  }
  pthread_mutex_lock(&memory_mutex);
#endif
  MEM_INFO *info = alloc_hashmap_get(ptr);
  if (info != NULL)
  {
    memory_allocated -= info->size;

    // Check the guard byte for corruption
    if (is_guard_valid(info) == 0)
    {
      if (VERBOSE)
        printf("Memory corruption detected at %p, file %s, line %d\n", ptr,
               file, line);
#ifdef MULTITHREAD
      pthread_mutex_unlock(&memory_mutex);
#endif
      return;
    }

    if (VERBOSE)
      printf("Freeing memory at %p, file %s, line %d\n", info->address, file,
             line);
    alloc_hashmap_remove(ptr);
    free(info->address); // Free at the original address
#ifdef MULTITHREAD
    pthread_mutex_unlock(&memory_mutex);
#endif
    return;
  }

  printf("Attempted to free unallocated memory at %p, file %s, line %d\n", ptr,
         file, line);
  abort();
#ifdef MULTITHREAD
  pthread_mutex_unlock(&memory_mutex);
#endif
}

void *realloc_debug(void *ptr, const size_t size, char *file, const int line)
{
#ifdef MULTITHREAD
  if (!memory_mutex_initialized)
  {
    pthread_mutex_init(&memory_mutex, NULL);
    memory_mutex_initialized = 1;
  }
  pthread_mutex_lock(&memory_mutex);
#endif
  if (ptr == NULL)
  {
    // MALLOC COPY (to reduce stack depth)
    void *address = malloc(size + sizeof(MEM_INFO));
    if (address)
    {
      MEM_INFO
      *info = (MEM_INFO *)(address + size); // Shift MEM_INFO to the end of
                                            // the allocated block
      memory_allocated += size;

      info->next = NULL;
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

      info->symbol = loc_hashmap_add(loc_name, line);

      alloc_hashmap_add(address, info);

      if (VERBOSE)
        printf("Allocated %zu bytes at %p, file %s, line %d\n", size,
               info->address, file, line);
#ifdef MULTITHREAD
      pthread_mutex_unlock(&memory_mutex);
#endif
      return info->address;
    }
    else
    {
#ifdef MULTITHREAD
      pthread_mutex_unlock(&memory_mutex);
#endif
      return NULL;
    }
    ////////////////////
  }

  if (size == 0)
  {
#ifdef MULTITHREAD
    pthread_mutex_unlock(&memory_mutex);
#endif
    free_debug(ptr, file, line);
    return NULL;
  }

  // Locate the existing memory block
  MEM_INFO *info = alloc_hashmap_get(ptr);
  if (info != NULL)
  {
    memory_allocated += size;

    // Check the guard byte for corruption
    if (is_guard_valid(info) == 0)
    {
      if (VERBOSE)
        printf("Memory corruption detected at %p, file %s, line %d\n", ptr,
               file, line);
#ifdef MULTITHREAD
      pthread_mutex_unlock(&memory_mutex);
#endif
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
      info->next = NULL;
      info->size = size;
      memset(info->guard, GUARD_CONTENT, GUARD_LENGTH); // Set guard bytes

      // store location info
      char *last_slash = strrchr(file, '/');
      if (last_slash == NULL)
        last_slash = file;
      else
        last_slash++;
      char *loc_name = last_slash;

      info->symbol = loc_hashmap_add(loc_name, line);

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
#ifdef MULTITHREAD
      pthread_mutex_unlock(&memory_mutex);
#endif
      free_debug(ptr, file, line);
    }

    return new_ptr;
  }

  printf("Attempted to reallocate unallocated memory at %p, file %s, line "
         "%d\n",
         ptr, file, line);
  abort();
#ifdef MULTITHREAD
  pthread_mutex_unlock(&memory_mutex);
#endif
  return NULL;
}

void memory_check_leaks()
{
  int leaks = 0;
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

      printf("-> Leaked %s%10lu%s bytes at %14p, from %s%8s%s line %d\n",
             COLOR_YELLOW, current->size, COLOR_DEFAULT, current->address,
             COLOR_LIGHT_BLUE, current->symbol->filename, COLOR_DEFAULT,
             current->symbol->line);
      leaks++;
      current = current->next;
    }
  }
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
}

void abort_trace()
{
  printf("%sAborting...%s\n", COLOR_RED, COLOR_DEFAULT);
#ifdef USE_BACKTRACE
  print_trace();
#endif
  abort();
}

void assert_debug(int condition, char *condition_str, char *file, int line)
{
  if (!condition)
  {
    printf("%sAssertion %s%s%s failed at %s%s line %d%s\n", COLOR_RED,
           COLOR_LIGHT_GREEN, condition_str, COLOR_RED, COLOR_MAGENTA, file,
           line, COLOR_DEFAULT);

    // Print stack trace
#ifdef USE_BACKTRACE
    abort_trace();
#endif

    abort();
  }
}

uint64_t memory_get_allocated()
{
  return memory_allocated;
}

void _print_debug(const char *file, const int line, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  const char *filename = strrchr(file, '/');
  filename = filename ? filename + 1 : file;
  printf("%s--> %-16s line %-5d   %s", COLOR_CYAN, filename, line,
         COLOR_LIGHT_CYAN);
  vprintf(format, args);
  printf("%s", COLOR_DEFAULT);
  va_end(args);
}
