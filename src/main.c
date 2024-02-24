#include "./micro/core/ECS.h"
#include "./micro/core/Graphics.h"
#include "./micro/core/State.h"
#include "./micro/core/Tilemap.h"
#include "./micro/util/debug.h"
#include <SDL2/SDL.h>

#include "states/GameState.h"
#include "states/TestState.h"
#include <string.h>

int main()
{
  srand(time(NULL));

  // Tilemap tm = tilemap_load_json("./res/tilemap.tmj");
  // debug_print("Tilemap width: %d\n", tm.width);
  // debug_print("Tilemap height: %d\n", tm.height);
  // debug_print("Tilemap tile_width: %d\n", tm.tile_width);
  // debug_print("Tilemap tile_height: %d\n", tm.tile_height);
  // debug_print("Tilemap num_layers: %d\n", tm.num_layers);
  // for (int i = 0; i < tm.num_layers; i++)
  // {
  //   debug_print("Layer %d: %s\n", i, tm.layers[i].name);
  //   if (tm.layers[i].type == TILE_LAYER)
  //   {
  //     TileLayer *layer = &tm.layers[i].tile_layer;
  //     for (int j = 0; j < tm.height; j++) {
  //       for (int k = 0; k < tm.width; k++) {
  //         printf("%d ", tilemap_get_tile_id(&tm, i, k, j));
  //       }
  //       printf("\n");
  //     }
  //   }
  //   else if (tm.layers[i].type == OBJECT_LAYER)
  //   {
  //     ObjectLayer *layer = &tm.layers[i].object_layer;
  //     for (int j = 0; j < layer->objects_count; j++)
  //     {
  //       Object *obj = &layer->objects[j];
  //       debug_print("Object %d: name: %s, x: %d, y: %d\n", j, obj->name ? obj->name : "null", obj->x, obj->y);
  //     }
  //   }
  // }
  // return 0;

  microGraphicsInit();
  microECSInit();
  // microStateSet(testStateGet());
  microStateSet(gameStateGet());

  float deltaTime = 0.0;
  while (1)
  {
    microStateUpdate(deltaTime);
    deltaTime = microGraphicsDelayToNextFrame(70);
  }

  microStateFree();
  microECSFree();
  microGraphicsQuit();

  return 0;
}
