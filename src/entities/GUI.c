#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../managers/inventory.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../micro/State.h"
#include "../util/vector.h"
#include "Player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ENERGY_BAR_FRAME_START_X 32
#define ENERGY_BAR_FRAME_START_Y 32
#define ENERGY_BAR_FRAME_WIDTH (96 * 3)
#define ENERGY_BAR_FRAME_HEIGHT (20 * 3)

#define ENERGY_BAR_START_X (ENERGY_BAR_FRAME_START_X + 22 * 3)
#define ENERGY_BAR_START_Y (ENERGY_BAR_FRAME_START_Y + 6 * 3)
#define ENERGY_BAR_WIDTH (70 * 3)
#define ENERGY_BAR_HEIGHT (8 * 3)

#define RESOURCES_SPACING 8
#define RESOURCES_START_X (32)
#define RESOURCES_START_Y (32 + 20 * 3 + RESOURCES_SPACING)
#define RESOURCES_ICON_SIZE 48
#define RESOURCES_ICON_OFFSET (2 * 3)
#define RESOURCES_TEXT_VOFFSET (32 + 2 * 3 - 2)
#define RESOURCES_TEXT_HOFFSET (64 + 16)
#define RESOURCES_FRAME_WIDTH (64 * 3)
#define RESOURCES_FRAME_HEIGHT (20 * 3)

int playerHealth = 0;
int playerMaxHealth = 0;
int hearthsCount = 0;
int hearthsMaxCount = 0;
char metal_count_text[16] = "0";
char crystal_count_text[16] = "0";
char deuterium_count_text[16] = "0";

uint32_t bar_frame_id = 0;
uint32_t bar_background_id = 0;
uint32_t bar_content_id = 0;

void GUIUpdate(int entity, float dt)
{
  (void)(entity); // Unused parameter
  (void)(dt);     // Unused parameter

  playerHealth = PlayerGetHealth();
  playerMaxHealth = PlayerGetMaxHealth();
  hearthsMaxCount = playerMaxHealth / 4;
  hearthsCount = playerHealth / 4;
  float normalized_energy = (float)playerHealth / (float)playerMaxHealth;

  // Update energy bar
  CTransform *bar_transform = (CTransform *)
    microECSEntityGetComponent(bar_content_id, cid_transform);
  bar_transform->width = (int)(normalized_energy * ENERGY_BAR_WIDTH);

  // Update inventory counters
  const int metal_count = inventoryGetCount(ITEM_METAL);
  const int crystal_count = inventoryGetCount(ITEM_CRYSTAL);
  const int deuterium_count = inventoryGetCount(ITEM_DEUTERIUM);
  sprintf(metal_count_text, "%d", metal_count);
  sprintf(crystal_count_text, "%d", crystal_count);
  sprintf(deuterium_count_text, "%d", deuterium_count);
}

uint32_t GUI_add_image(uint32_t atlasId, uint32_t x, uint32_t y,
                       char *image_name, uint32_t width, uint32_t height,
                       uint32_t origin_x, uint32_t origin_y)
{
  const int textureId = microTextureAtlasGetTextureId(atlasId);
  const MicroTextureSource txs = microTextureAtlasGetRegion(atlasId,
                                                            image_name);
  int entity_id = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entity_id, cid_position,
                             &(CPosition){.x = x, .y = y});

  microECSEntityAddComponent(entity_id, cid_sprite,
                             &(CSprite){
                               .textureId = textureId,
                               .tx = txs.x,
                               .ty = txs.y,
                               .tw = txs.w,
                               .th = txs.h,
                             });

  // Transform component
  microECSEntityAddComponent(entity_id, cid_transform,
                             &(CTransform){
                               .width = width,
                               .height = height,
                               .originX = origin_x,
                               .originY = origin_y,
                               .rotation = 0.0,
                             });

  microECSEntityAddComponent(entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = 5,
                               .visible = 1,
                             });
  microECSEntityAddComponent(entity_id, cid_hud, NULL);

  return entity_id;
}
uint32_t GUI_add_text(uint32_t x, uint32_t y, char *text)
{
  int entity_id = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entity_id, cid_position,
                             &(CPosition){.x = x, .y = y});

  microECSEntityAddComponent(entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = 6,
                               .visible = 1,
                             });
  microECSEntityAddComponent(entity_id, cid_hud, NULL);

  uint32_t font_id = microResourceGet("ui_font");

  microECSEntityAddComponent(entity_id, cid_text,
                             &(CText){
                               .text = text,
                               .lineSpacing = 3,
                               .fontId = font_id,
                             });

  return entity_id;
}

void GUIFree(int entityId)
{
  (void)(entityId); // Unused parameter
}

void GUIInit()
{
  // Create handler for updating life UI and freeing memory
  int gui_handler = microECSEntityNew(NULL, GUIFree);
  microECSEntityAddComponent(gui_handler, cid_update,
                             &(CUpdate){.update = GUIUpdate});

  // Creating and computing lifes
  playerHealth = PlayerGetHealth();
  playerMaxHealth = PlayerGetMaxHealth();
  hearthsMaxCount = playerMaxHealth / 4;
  hearthsCount = playerHealth / 4;

  const int atlasId = microResourceGet("atlas");

  // Add energy bar
  bar_background_id = GUI_add_image(atlasId, ENERGY_BAR_START_X,
                                    ENERGY_BAR_START_Y, "energy_bar_background",
                                    ENERGY_BAR_WIDTH, ENERGY_BAR_HEIGHT, 0, 0);

  bar_content_id = GUI_add_image(atlasId, ENERGY_BAR_START_X,
                                 ENERGY_BAR_START_Y, "energy_bar_content",
                                 ENERGY_BAR_WIDTH, ENERGY_BAR_HEIGHT, 0, 0);

  bar_frame_id = GUI_add_image(atlasId, ENERGY_BAR_FRAME_START_X,
                               ENERGY_BAR_FRAME_START_Y, "energy_bar_frame",
                               ENERGY_BAR_FRAME_WIDTH, ENERGY_BAR_FRAME_HEIGHT,
                               0, 0);

  // Add metal counter
  GUI_add_image(atlasId, RESOURCES_START_X, RESOURCES_START_Y, "UI_counter",
                RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0, 0);

  GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
                RESOURCES_START_Y + RESOURCES_ICON_OFFSET, "metal",
                RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);

  GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
               RESOURCES_START_Y + RESOURCES_TEXT_VOFFSET, metal_count_text);

  // Add crystal counter
  GUI_add_image(atlasId, RESOURCES_START_X,
                RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT + RESOURCES_SPACING,
                "UI_counter", RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0,
                0);

  GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
                RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 1 +
                  RESOURCES_SPACING + RESOURCES_ICON_OFFSET,
                "crystal", RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);

  GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
               RESOURCES_START_Y + RESOURCES_SPACING +
                 RESOURCES_FRAME_HEIGHT * 1 + RESOURCES_TEXT_VOFFSET,
               crystal_count_text);

  // Add deuterium counter
  GUI_add_image(atlasId, RESOURCES_START_X,
                RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 2 +
                  RESOURCES_SPACING * 2,
                "UI_counter", RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0,
                0);

  GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
                RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 2 +
                  RESOURCES_SPACING * 2 + RESOURCES_ICON_OFFSET,
                "deuterium", RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);

  GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
               RESOURCES_START_Y + RESOURCES_SPACING * 2 +
                 RESOURCES_FRAME_HEIGHT * 2 + RESOURCES_TEXT_VOFFSET,
               deuterium_count_text);
}
