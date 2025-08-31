#ifndef STRDICT_H
#define STRDICT_H

#include <stdint.h>
#include <string.h>

typedef struct StrDictEntry {
  char *key;
  int value;
} StrDictEntry;

typedef struct StrDict {
  int size;
  StrDictEntry entries[];
} StrDict;

int strdict_get(const StrDictEntry *dict, const char *key, const int dict_size);
int strdict_insert(StrDictEntry *dict, const char *key, const int value,
                   const int dict_size);
int strdict_remove(StrDictEntry *dict, const char *key, const int dict_size);

#endif /* end of include guard: STRDICT_H */

