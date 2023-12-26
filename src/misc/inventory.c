#include "../components/CustomComponents.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../util/debug.h"
#include "items.h"
#include <stdint.h>

int32_t inventory_pickup_sound = -1;
uint32_t inventory_max_space = 10000000;
uint32_t inventory[ITEM_COUNT] = {0};
uint8_t inventory_visible = 1;

void inventorySetPickupSound(uint32_t sound_id)
{
  inventory_pickup_sound = sound_id;
}

int32_t inventoryAdd(uint32_t entity_id)
{
  if (microECSEntityHasComponent(entity_id, cid_item) == 0)
    return 0; // not valid item

  CItem *item = (CItem *)microECSEntityGetComponent(entity_id, cid_item);

  if (item->type >= ITEM_COUNT)
    return 0; // not valid item

  if (inventory[item->type] + item->quantity > inventory_max_space)
  {
    item->quantity = inventory_max_space - inventory[item->type];
    inventory[item->type] = inventory_max_space;
    return -1; // not enough space
  }

  inventory[item->type] += item->quantity;
  return 1; // success
}

void inventoryPickupCallback(uint32_t entity_id)
{
  int32_t r = inventoryAdd(entity_id);
  if (r == 1)
  {
    microECSEntityRemove(entity_id);
    if (inventory_pickup_sound != -1)
      microSoundPlay(inventory_pickup_sound, 0);
  }
}

int32_t inventoryRemove(uint32_t item_type, uint32_t quantity)
{
  if (item_type >= ITEM_COUNT)
    return 0; // not valid item

  if (inventory[item_type] < quantity)
    return -1; // not enough items

  inventory[item_type] -= quantity;
  return 1; // success
}

uint32_t inventoryGetCount(uint32_t item_type)
{
  assert(item_type < ITEM_COUNT);
  return inventory[item_type];
}

uint32_t inventoryIsVisible()
{
  return inventory_visible;
}
