#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

////////////////////////////////////
////// VECTOR IMPLEMENTATION ///////
////////////////////////////////////
typedef struct {
    void** data;
    size_t size;
    size_t capacity;
    size_t dataSize;
} Vector;

extern Vector vector_create(unsigned int dataSize);
extern void vector_free(Vector* vec);
extern void vector_push_back(Vector* vec, void* value);
extern void vector_pop_back(Vector* vec);
extern void* vector_back(Vector* vec);

#endif /* end of include guard: VECTOR_H */
