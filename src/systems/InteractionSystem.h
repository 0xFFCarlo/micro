#ifndef INTERACTION_SYSTEM_H
#define INTERACTION_SYSTEM_H

#include <stdint.h>

// Interaction system update function (called by the engine)
extern void interactionSystem(float dt);

// Set the actor (player) entity id for the interaction system
extern void interactionSystemSetActorId(uint32_t entity_id);

// Return entity id that is in range of the player
// and has the CInteractive component. Returns -1 if
// no entity is in range.
extern int32_t interactionSystemGetObjectId();

// Request to interact with the object in range
// Returns 1 if successful, 0 if not
extern uint8_t interactionSystemInteract();

#endif /* end of include guard: INTERACTION_SYSTEM_H */
