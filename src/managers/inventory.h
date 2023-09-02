#ifndef INVENTORY_H
#define INVENTORY_H

#include "items.h"
#include <stdint.h>

extern void inventorySetPickupSound(uint32_t sound_id);

// Try to add an item to the inventory
// Returns 1 if successful, 0 if not valid item and -1 if not enough space
extern int32_t inventoryAdd(uint32_t entity_id);

// Pickup callback for the interaction system
extern void inventoryPickupCallback(uint32_t entity_id);

// Remove an item from the inventory
// Returns 1 if successful, 0 if item not found and -1 if not enough quantity
extern int32_t inventoryRemove(uint32_t item_type, uint32_t quantity);

// Get the quantity of an item in the inventory
extern uint32_t inventoryGetCount(uint32_t item_type);

// Check if the inventory is visible
extern uint32_t inventoryIsVisible();

#endif // INVENTORY_H
