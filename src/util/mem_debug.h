#ifndef MEM_DEBUG_H
#define MEM_DEBUG_H

#define DEBUG_MODE 1

#include <stddef.h>
#include <stdint.h>

#if DEBUG_MODE == 1
extern void *malloc_debug(size_t size, char *file, int line);
extern void *realloc_debug(void *ptr, size_t size, char *file, int line);
extern void free_debug(void *ptr, char *file, int line);
extern void memory_check_leaks();
extern void memory_check_corruption();

#define malloc(size) malloc_debug(size, __FILE__, __LINE__)
#define realloc(ptr, size) realloc_debug(ptr, size, __FILE__, __LINE__)
#define free(ptr) free_debug(ptr, __FILE__, __LINE__)

#else

// Do nothing
#define memory_check_leaks()
#define memory_check_corruption()

#endif

#endif
