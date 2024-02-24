#include "Resource.h"
#include "../components/CustomComponents.h"
#include "../micro/components/LogicComponents.h"
#include "../micro/components/MotionComponents.h"
#include "../micro/components/RenderingComponents.h"
#include "../micro/core/ECS.h"
#include "../micro/core/Graphics.h"
#include "../micro/core/Physics.h"
#include "../micro/core/Resources.h"
#include "../misc/inventory.h"
#include "../misc/layers.h"
#include "Planet.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int ResourceAddEntity(const int x, const int y, const int type)
{
  int res_entity_id = microECSEntityNew(NULL, NULL);
  assert(res_entity_id != -1);

  // Position component
  CmpAddPosition(res_entity_id, x, y);

  // Sprite component
  int atlasId = microResourceGet("atlas");
  int textureId = microTextureAtlasGetTextureId(atlasId);
  MicroTextureSource ts;
  if (type == RES_METAL)
    ts = microTextureAtlasGetRegion(atlasId, "metal");
  else if (type == RES_CRYSTAL)
    ts = microTextureAtlasGetRegion(atlasId, "crystal");
  else if (type == RES_DEUTERIUM)
    ts = microTextureAtlasGetRegion(atlasId, "deuterium");
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  CmpAddSprite(res_entity_id, textureId, ts.x, ts.y, ts.w, ts.h);

  CmpAddTransform(res_entity_id, 32, 32, 32 / 2.0, 32 / 2.0, 0.0);
  CmpAddColor(res_entity_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(res_entity_id, LAYER_GROUND_1, TRUE);
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);
  CmpAddPlanetaryAlignment(res_entity_id, planetX, planetY);
  CmpAddInteractive(res_entity_id, 32 + 2, "Pick up", inventoryPickupCallback);
  if (type == RES_METAL)
    CmpAddItem(res_entity_id, ITEM_METAL, 1);
  else if (type == RES_CRYSTAL)
    CmpAddItem(res_entity_id, ITEM_CRYSTAL, 1);
  else if (type == RES_DEUTERIUM)
    CmpAddItem(res_entity_id, ITEM_DEUTERIUM, 1);

  return res_entity_id;
}
