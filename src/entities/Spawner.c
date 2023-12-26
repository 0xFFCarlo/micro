#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/Audio.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../misc/collision.h"
#include "Drone.h"
#include "DroneLander.h"
#include "Planet.h"
#include "Player.h"
#include "../util/vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int spawner_eid = -1;
float spawner_timer = 0.0;
bool spawner_active = FALSE;
Vector spawner_enemies;


void SpawnerUpdate(int spawner_eid, float dt)
{
  (void)spawner_eid;

  // Check if any enemies are dead
  uint32_t i = spawner_enemies.size;
  while (i--)
  {
    const int enemy_eid = *(int*)vector_at(&spawner_enemies, i);
    if (!microECSEntityIsAlive(enemy_eid))
      vector_remove(&spawner_enemies, i);
  }

  if (!spawner_active)
    return;

  spawner_timer += dt;
  if (spawner_timer > 5)
  {
    spawner_timer = 0.0;
    float rand_angle = (rand() % 360) * (M_PI / 180.0);
    int spawn_x, spawn_y;
    PlanetGetSurfacePosition(rand_angle, -500.0, &spawn_x, &spawn_y);
    
    int enemy_eid;
    if (rand() % 2 == 0)
      enemy_eid = DroneAddEntity(spawn_x, spawn_y);
    else
      enemy_eid = DroneLanderAddEntity(spawn_x, spawn_y);
    vector_push_back(&spawner_enemies, &enemy_eid);
  }
}

void SpawnerFree(int spawner_eid)
{
  (void)spawner_eid;
  vector_free(&spawner_enemies);
}

void SpawnerStart()
{
  // Create spawner if it doesn't exist
  if (spawner_eid == -1) {
    spawner_eid = microECSEntityNew(NULL, SpawnerFree);
    assert(spawner_eid != -1);
    CmpAddUpdate(spawner_eid, SpawnerUpdate);
    spawner_enemies = vector_create(sizeof(int));
  }

  spawner_timer = 0.0;
  spawner_active = TRUE;
}

void SpawnerStop()
{
  spawner_active = FALSE;
}

int SpawnerIsActive()
{
  return spawner_active;
}

void SpawnerClear()
{
  
  // Remove all enemies
  uint32_t i = spawner_enemies.size;
  while (i--)
  {
    const int enemy_eid = *(int*)vector_at(&spawner_enemies, i);
    if (microECSEntityIsAlive(enemy_eid)) {
      microECSEntityRemove(enemy_eid);
    }
  }

  vector_clear(&spawner_enemies);
}
