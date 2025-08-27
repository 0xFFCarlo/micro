#include "vector.h"
#include "debug.h"
#include <stddef.h> // offsetof
#include <stdint.h> // SIZE_MAX
#include <string.h>

static inline int mul_overflow_size(size_t a, size_t b, size_t *out)
{
  if (a == 0 || b == 0)
  {
    *out = 0;
    return 0;
  }
  if (a > SIZE_MAX / b)
    return 1;
  *out = a * b;
  return 0;
}

void *vec_new(size_t elem_size)
{
  const size_t init_cap = VEC_INIT_CAPACITY;
  if (elem_size == 0)
    return NULL;

  size_t bytes;
  if (mul_overflow_size(elem_size, init_cap, &bytes))
    return NULL;

  _vec_hdr_t *h = (_vec_hdr_t *)malloc(sizeof *h + bytes);
  if (!h)
    return NULL;

  h->elem_size = elem_size;
  h->capacity = init_cap;
  h->count = 0;
  return h->data;
}

void vec_free(void *vec)
{
  if (!vec)
    return;
  _vec_hdr_t *h = VEC_HDR(vec);
  free(h);
}

static int grow_if_needed(void **vec, size_t needed_count)
{
  _vec_hdr_t *h = VEC_HDR(*vec);
  if (needed_count <= h->capacity)
    return 0;

  size_t new_cap = h->capacity ? h->capacity : 1;
  while (new_cap < needed_count)
  {
    if (new_cap > SIZE_MAX / 2)
      return -1; // would overflow
    new_cap *= 2;
  }

  size_t bytes;
  if (mul_overflow_size(h->elem_size, new_cap, &bytes))
    return -1;

  _vec_hdr_t *nh = (_vec_hdr_t *)realloc(h, sizeof *nh + bytes);
  if (!nh)
    return -1;

  nh->capacity = new_cap;
  *vec = nh->data;
  return 0;
}

int _vec_append(void **vec, const void *elem)
{
  if (!vec || !*vec || !elem)
    return -1;

  _vec_hdr_t *h = VEC_HDR(*vec);
  if (grow_if_needed(vec, h->count + 1))
    return -1;
  h = VEC_HDR(*vec); // may have moved

  void *dst = h->data + h->count * h->elem_size;
  memcpy(dst, elem, h->elem_size);
  h->count++;
  return 0;
}

// TODO: maybe bugged
int _vec_remove_at(void **vec, size_t index)
{
  if (!vec || !*vec)
    return -1;
  _vec_hdr_t *h = VEC_HDR(*vec);
  if (index >= h->count)
    return -1;

  size_t last = h->count - 1;
  if (index != last)
  {
    void *dst = h->data + index * h->elem_size;
    void *src = h->data + last * h->elem_size;
    memcpy(dst, src, h->elem_size); // unordered erase
  }
  h->count = last;

  // Best-effort shrink; not an error if it fails
  const size_t min_cap = 8;
  if (h->capacity > min_cap && h->count < h->capacity / 4)
  {
    size_t new_cap = h->capacity / 2;
    if (new_cap < min_cap)
      new_cap = min_cap;

    size_t bytes;
    if (!mul_overflow_size(h->elem_size, new_cap, &bytes))
    {
      _vec_hdr_t *nh = (_vec_hdr_t *)realloc(h, sizeof *nh + bytes);
      if (nh)
      {
        nh->capacity = new_cap;
        *vec = nh->data;
      }
    }
  }
  return 0;
}

void _vec_pop_back(void **vec)
{
  if (!vec || !*vec)
    return;
  _vec_hdr_t *h = VEC_HDR(*vec);
  if (h->count == 0)
    return;
  _vec_remove_at(vec, h->count - 1);
}

int _vec_remove(void **vec, const void *elem)
{
  if (!vec || !*vec || !elem)
    return -1;
  _vec_hdr_t *h = VEC_HDR(*vec);
  for (size_t i = 0; i < h->count; ++i)
  {
    void *cur = h->data + i * h->elem_size;
    if (memcmp(cur, elem, h->elem_size) == 0)
      return _vec_remove_at(vec, i);
  }
  return -1; // not found
}

void *vec_back(void *vec)
{
  if (!vec)
    return NULL;
  const _vec_hdr_t *h = VEC_HDR((void *)vec);
  if (h->count == 0)
    return NULL;
  return (void *)(h->data + (h->count - 1) * h->elem_size);
}

void _vec_clear(void **vec)
{
  if (!vec || !*vec)
    return;
  _vec_hdr_t *h = VEC_HDR(*vec);
  h->count = 0;

  if (h->capacity == VEC_INIT_CAPACITY)
    return; // nothing to do

  size_t bytes;
  if (mul_overflow_size(h->elem_size, VEC_INIT_CAPACITY, &bytes))
    return;

  _vec_hdr_t *nh = (_vec_hdr_t *)realloc(h, sizeof *nh + bytes);
  if (nh)
  {
    nh->capacity = VEC_INIT_CAPACITY;
    *vec = nh->data; // update user pointer if moved
  }
}

// Clear vector without resizing
void _vec_empty(void **vec)
{
  if (!vec || !*vec)
    return;
  _vec_hdr_t *h = VEC_HDR(*vec);
  h->count = 0; // just reset count, keep capacity
}
