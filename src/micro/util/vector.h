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
Vector vector_create(unsigned int data_size);

// create a new vector with a specified capacity
Vector vector_create_with_capacity(unsigned int data_size, unsigned int capacity);

// free the vector and all its elements
void vector_free(Vector *vec);

// push an element to the end of the vector
void vector_push_back(Vector *vec, const void *value);

// remove the last element
void vector_pop_back(Vector *vec);

// remove the element at the specified index
// last element is moved to the position of the removed element
void vector_remove_at(Vector *vec, const unsigned int index);

// remove the element with the specified value
// the first occurrence of the value is removed
void vector_remove_val(Vector *vec, const void *value);

// return the first element
void *vector_back(Vector *vec);

// return the element at the specified index
void *vector_at(Vector *vec, const unsigned int index);

// return the last element
void *vector_last(Vector *vec);

// remove all elements from the vector and reallocate memory
void vector_clear(Vector *vec);

// remove all elements from the vector but do not reallocate memory
void vector_empty(Vector *vec);

#endif /* end of include guard: VECTOR_H */
