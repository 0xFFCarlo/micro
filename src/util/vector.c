#include "vector.h"
#include "debug.h"
#include <assert.h>
#include <memory.h>

Vector vector_create(unsigned int data_size)
{
  Vector vec;
  vec.data = malloc(data_size * 2); // Initial capacity is 2
  vec.size = 0;
  vec.data_size = data_size;
  vec.capacity = 2;
  return vec;
}

void vector_free(Vector *vec)
{
  free(vec->data);
  vec->size = 0;
}

void vector_push_back(Vector *vec, void *value)
{
  if ((vec->size + 1) >= vec->capacity)
  {
    // We need to increase the capacity
    vec->capacity *= 2;
    vec->data = realloc(vec->data, vec->data_size * vec->capacity);
  }
  memcpy((char *)vec->data + vec->size * vec->data_size, value, vec->data_size);
  vec->size++;
}

void vector_pop_back(Vector *vec)
{
  if (vec->size > 0)
    vec->size--;

  if (vec->size <= vec->capacity / 4)
  {
    // We need to decrease the capacity
    vec->capacity /= 2;
    vec->data = realloc(vec->data, vec->data_size * vec->capacity);
  }
}

void *vector_back(Vector *vec)
{
  if (vec->size > 0)
    return (char *)vec->data + (vec->size - 1) * vec->data_size;
  return NULL; // Return NULL if vector is empty.
}

void *vector_at(Vector *vec, unsigned int index)
{
  assert(index < (unsigned int)vec->size);
  return (char *)vec->data + index * vec->data_size;
}

void vector_clear(Vector *vec)
{
  vec->size = 0;
  vec->capacity = 2;
  vec->data = realloc(vec->data, vec->data_size * vec->capacity);
}

void vector_empty(Vector *vec)
{
  vec->size = 0;
}
