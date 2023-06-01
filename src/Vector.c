#include "Vector.h"
#include <memory.h>

Vector vector_create(unsigned int dataSize) {
    Vector vec;
    vec.data = malloc(dataSize * 2); // Initial capacity is 2
    vec.size = 0;
    vec.dataSize = dataSize;
    vec.capacity = 2;
    return vec;
}

void vector_free(Vector* vec) {
    free(vec->data);
    vec->size = 0;
}

void vector_push_back(Vector* vec, void* value) {
    if (vec->size >= vec->capacity) {
        // We need to increase the capacity
        vec->capacity *= 2;
        vec->data = realloc(vec->data, vec->dataSize * vec->capacity);
    }
    memcpy((char*)vec->data + vec->size * vec->dataSize, value, vec->dataSize);
    vec->size++;
}

void vector_pop_back(Vector* vec) {
    if (vec->size > 0)
        vec->size--;
    
    if (vec->size <= vec->capacity / 4) {
        // We need to decrease the capacity
        vec->capacity /= 2;
        vec->data = realloc(vec->data, vec->dataSize * vec->capacity);
    }
}

void* vector_back(Vector* vec) {
    if (vec->size > 0)
        return (char*)vec->data + (vec->size - 1) * vec->dataSize;
    return NULL; // Return NULL if vector is empty.
}
