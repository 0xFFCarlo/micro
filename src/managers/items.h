#ifndef ITEMS_H
#define ITEMS_H

#include <stdint.h>

typedef enum ItemType
{
  ITEM_METAL,
  ITEM_CRYSTAL,
  ITEM_DEUTERIUM,
  ITEM_COUNT
} ItemType;

static char *item_names[] = {"Metal", "Crystal", "Deuterium"};

#endif
