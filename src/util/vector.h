/*
 * This file provides a generic implementation of a dynamic array (often
 * referred to as a vector in many languages) in C. It is capable of storing
 * elements of any type. Here is a summary of the data structure and functions:
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct
{
  void **data;
  size_t size;
  size_t capacity;
  size_t data_size;
} Vector;

// create a new vector
extern Vector vector_create(unsigned int data_size);

// free the vector and all its elements
extern void vector_free(Vector *vec);

// push an element to the end of the vector
extern void vector_push_back(Vector *vec, void *value);

// remove the last element
extern void vector_pop_back(Vector *vec);

// return the first element
extern void *vector_back(Vector *vec);

// return the element at the specified index
extern void *vector_at(Vector *vec, unsigned int index);

// remove all elements from the vector and reallocate memory
extern void vector_clear(Vector *vec);

// remove all elements from the vector but do not reallocate memory
extern void vector_empty(Vector *vec);

#endif /* end of include guard: VECTOR_H */
