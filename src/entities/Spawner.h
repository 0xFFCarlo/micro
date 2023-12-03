#ifndef SPAWNER_H
#define SPAWNER_H

// Makes spawner active and starts spawning enemies
extern void SpawnerStart();

// Stops spawning enemies
extern void SpawnerStop();

// Returns true if spawner is active
extern int SpawnerIsActive();

// Removes all enemies from the game
extern void SpawnerClear();

#endif
