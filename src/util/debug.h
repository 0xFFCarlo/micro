#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_MEMORY 1
#define DEBUG_PRINTS 1

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if DEBUG_MEMORY == 1
extern void *malloc_debug(const size_t size, char *file, const int line);
extern void *realloc_debug(void *ptr, const size_t size, char *file,
                           const int line);
extern void free_debug(void *ptr, const char *file, const int line);
extern void memory_check_leaks();
extern void memory_check_corruption();
extern void assert_debug(int condition, char *condition_str, char *file,
                         int line);

// Redefine malloc, realloc, free and assert
#undef malloc
#undef realloc
#undef free
#undef assert
#define malloc(size) malloc_debug(size, __FILE__, __LINE__)
#define realloc(ptr, size) realloc_debug(ptr, size, __FILE__, __LINE__)
#define free(ptr) free_debug(ptr, __FILE__, __LINE__)
#define assert(condition)                                                      \
  assert_debug(condition, #condition, __FILE__, __LINE__)

#else

#include <assert.h>

// Do nothing
#define memory_check_leaks()
#define memory_check_corruption()

#endif // DEBUG_MEMORY

#if DEBUG_PRINTS == 1
extern void _print_debug(const char *file, const int line, const char *format,
                         ...);

#define debug_print(format, ...)                                               \
  _print_debug(__FILE_NAME__, __LINE__, format, ##__VA_ARGS__)

#else

// Do nothing
#define debug_printf(format, ...)

#endif // DEBUG_PRINTS

#endif // DEBUG_H
