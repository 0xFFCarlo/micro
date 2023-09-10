#include "Resource.h"
#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../managers/inventory.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void ResourceAddEntity(const int x, const int y)
{
  int res_entity_id = microECSEntityNew(NULL, NULL);
  assert(res_entity_id != -1);

  // Position component
  CmpAddPosition(res_entity_id, x, y);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "metal");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  CmpAddSprite(res_entity_id, textureId, ts.x, ts.y, ts.w, ts.h);

  CmpAddTransform(res_entity_id, 32, 32, 32 / 2.0, 32 / 2.0, 0.0);
  CmpAddColor(res_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(res_entity_id, 4, 1);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(res_entity_id, planetX, planetY);
  CmpAddInteractive(res_entity_id, 32 + 2, "Pick up", inventoryPickupCallback);
  CmpAddItem(res_entity_id, ITEM_METAL, 1);
}
