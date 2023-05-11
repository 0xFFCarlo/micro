#include "../components/CPosition.h"
#include "../components/CSprite.h"
#include "../Resources.h"
#include "../Graphics.h"
#include "../ECS.h"
#include <stdio.h>


int planet_id;

void PlanetEntityAdd()
{
  planet_id = microECSEntityNew();

  CPosition position = {
    .x = 100,
    .y = 100,
  };
  microECSEntityAddComponent(planet_id, cid_position, &position);
  
 
  int texture_id = microTextureLoadFromFile("./res/skull.png");
  microResourceGet("skull");
  microTexttureSetFilter(texture_id, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(texture_id, &texWidth, &texHeight);
  CSprite sprite = {
    .textureId = texture_id,
    .tx = 0,
    .ty = 0,
    .tw = texWidth,
    .th = texHeight,
    .width = 256,
    .height = 256,
    .originX = 0,
    .originY = 0,
    .rotation = 20,
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0
  };
  microECSEntityAddComponent(planet_id, cid_sprite, &sprite);
}
