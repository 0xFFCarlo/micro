#include "Resource.h"
#include "Planet.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/CustomComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

void ResourceAddEntity(const int x, const int y)
{
  int res_entity_id = microECSEntityNew(NULL, NULL); 
  assert(res_entity_id != -1);

  // Position component
  microECSEntityAddComponent(res_entity_id, cid_position, &(CPosition){
    .x = x,
    .y = y,
  });
  
  // Sprite component
  //int textureId = microTextureLoadFromFile("./res/robot.png");
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "iron-1");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);

  microECSEntityAddComponent(res_entity_id, cid_sprite, &(CSprite){
    .textureId = textureId,
    .tx = ts.x,
    .ty = ts.y,
    .tw = ts.w,
    .th = ts.h,
  });

  // Transform component
  microECSEntityAddComponent(res_entity_id, cid_transform, &(CTransform){
    .width = 24,
    .height = 24,
    .originX = 24/2.0,
    .originY = 24/2.0,
    .rotation = 0.0,
  });

  // Color component
  microECSEntityAddComponent(res_entity_id, cid_color, &(CColor){
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0,
  });

  // Layer component
  microECSEntityAddComponent(res_entity_id, cid_drawable, &(CDrawable){
    .layerId = 4,
    .visible = 1,
  });

  // Planetary alignment component
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  microECSEntityAddComponent(res_entity_id, cid_planetary_alignment, &(CPlanetaryAlignment) {
    .planet_x = planetX,
    .planet_y = planetY,
  });
  
  // Body component
  // int res_body_id = microPhysicsBodyNewCircle(0, x, y, 24/2.0, 1.0, 0, 1.0, 0.0, 1.0);
  // microECSEntityAddComponent(res_entity_id, cid_body, &(CBody) {
  //     .body_id = res_body_id,
  // });
}
