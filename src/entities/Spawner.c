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
#include "Planet.h"
#include "Player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float timer = 0.0;

void SpawnerUpdate(int spawner_eid, float dt)
{
  (void)spawner_eid;
  timer += dt;
  if (timer > 5) {
    timer = 0.0;
    float rand_angle = (rand() % 360) * (M_PI / 180.0);
    int spawn_x, spawn_y;
    PlanetGetSurfacePosition(rand_angle, -500.0, &spawn_x, &spawn_y);
    DroneAddEntity(spawn_x, spawn_y);
  }
}

void SpawnerEntityAdd()
{
  int spawner_eid = microECSEntityNew(NULL, NULL);
  assert(spawner_eid != -1);
  CmpAddUpdate(spawner_eid, SpawnerUpdate);
}
