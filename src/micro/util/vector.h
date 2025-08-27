#ifndef VECTOR_H
#define VECTOR_H

#include <stdalign.h> // alignas, max_align_t
#include <stddef.h>
#include <stdint.h>

#define VEC_INIT_CAPACITY 8

typedef struct _vec_hdr_t
{
  size_t elem_size;                          // bytes per element
  size_t capacity;                           // allocated elements
  size_t count;                              // used elements
  alignas(max_align_t) unsigned char data[]; // start of storage
} _vec_hdr_t;

#define VEC_HDR(p) ((_vec_hdr_t *)((char *)(p)-offsetof(_vec_hdr_t, data)))

// Allocate a new vector with the specified element size
void *vec_new(size_t elem_size);

// Free the vector and its memory
void vec_free(void *vec);

// Get length of the vector
#define vec_len(vec) ((vec) ? VEC_HDR(vec)->count : 0)

// Get capacity of the vector
#define vec_capacity(vec) ((vec) ? VEC_HDR(vec)->capacity : 0)

// Get element size of the vector
#define vec_elem_size(vec) ((vec) ? VEC_HDR(vec)->elem_size : 0)

// Add element to the end of the vector
#define vec_append(vec, elem) _vec_append((void **)(&vec), (const void *)(elem))
int _vec_append(void **vec, const void *elem);

// Remove last element from vector
#define vec_pop_back(vec) _vec_pop_back((void **)(&vec))
void _vec_pop_back(void **vec);

// Remove element at index from vector
#define vec_remove_at(vec, index) _vec_remove_at((void **)(&vec), (index))
int _vec_remove_at(void **vec, size_t index);

// Remove element from vector by value
#define vec_remove(vec, elem) _vec_remove((void **)(&vec), (const void *)(elem))
int _vec_remove(void **vec, const void *elem);

// Get last element of the vector
void *vec_back(void *vec);

// Clear vector and resize to initial capacity
#define vec_clear(vec) _vec_clear((void **)(&vec))
void _vec_clear(void **vec);

// Clear vector without resizing
#define vec_empty(vec) _vec_empty((void **)(&vec))
void _vec_empty(void **vec);

#endif /* end of include guard: VECTOR_H */
