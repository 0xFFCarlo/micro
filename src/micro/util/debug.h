// Created by: Carlo meroni
//
// README:
// Does not work with strdup and will crash if you try to free a string
// allocated with strdup. Use malloc and strcpy instead.

#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_MEMORY 1
#define DEBUG_PRINTS 1
#define DEBUG_ASSERT 1

#include <stdint.h>
#include <stdlib.h>

#if DEBUG_MEMORY == 1
void *malloc_debug(const size_t size, char *file, const int line);
void *realloc_debug(void *ptr, const size_t size, char *file,
                           const int line);
void free_debug(void *ptr, const char *file, const int line);
void memory_check_leaks();
void memory_check_corruption();
void assert_debug(int condition, char *condition_str, char *file,
                         int line);
uint64_t memory_get_allocated();

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

#if (DEBUG_ASSERT == 1)
#include <assert.h>
#else
#define assert(condition) 0
#endif // DEBUG_ASSERT

// Do nothing
#define memory_check_leaks()
#define memory_check_corruption()
#define memory_get_allocated() 0ULL

#endif // DEBUG_MEMORY

#if DEBUG_PRINTS == 1
void _print_debug(const char *file, const int line, const char *format,
                         ...);

#define debug_print(format, ...)                                               \
  _print_debug(__FILE_NAME__, __LINE__, format, ##__VA_ARGS__)

#else

// Do nothing
#define debug_print(format, ...) ((void)0)

#endif // DEBUG_PRINTS

#endif // DEBUG_H
