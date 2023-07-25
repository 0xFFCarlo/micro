#ifndef VECTOR_H
#define VECTOR_H

////////////////////////////////////
////// VECTOR IMPLEMENTATION ///////
////////////////////////////////////
typedef struct
{
  void **data;
  unsigned int size;
  unsigned int capacity;
  unsigned int dataSize;
} Vector;

extern Vector vector_create(unsigned int dataSize);
extern void vector_free(Vector *vec);
extern void vector_push_back(Vector *vec, void *value);
extern void vector_pop_back(Vector *vec);
extern void *vector_back(Vector *vec);
extern void *vector_at(Vector *vec, unsigned int index);

#endif /* end of include guard: VECTOR_H */
