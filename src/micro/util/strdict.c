#include "strdict.h"

static unsigned int hash_djb2(const char *str)
{
  unsigned int hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash;
}

int strdict_get(const StrDictEntry *dict, const char *key, const int dict_size)
{
  const int k = hash_djb2(key) % dict_size;
  for (int i = 0; i < dict_size; i++)
  {
    const int index = (k + i) % dict_size;
    if (dict[index].key == NULL)
      return -1; // Not found
    if (strcmp(dict[index].key, key) == 0)
      return dict[index].value;
  }
  return -1; // Not found
}

int strdict_insert(StrDictEntry *dict, const char *key, const int value,
                   const int dict_size)
{
  const int k = hash_djb2(key) % dict_size;
  for (int i = 0; i < dict_size; i++)
  {
    const int index = (k + i) % dict_size;
    if (dict[index].key != NULL)
      continue;
    dict[index].key = (char *)key;
    dict[index].value = value;
    return index;
  }
  return -1; // No space left
}

int strdict_remove(StrDictEntry *dict, const char *key, const int dict_size)
{
  const int k = hash_djb2(key) % dict_size;
  for (int i = 0; i < dict_size; i++)
  {
    const int index = (k + i) % dict_size;
    if (dict[index].key == NULL)
      return -1; // Not found
    if (strcmp(dict[index].key, key) == 0)
    {
      dict[index].key = NULL;
      return index;
    }
  }
  return -1; // Not found
}
