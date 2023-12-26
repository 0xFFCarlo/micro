#include "../components/CustomComponents.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../micro/State.h"
#include "../micro/System.h"
#include "../misc/inventory.h"
#include "../misc/layers.h"
#include "../systems/InteractionSystem.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include "Player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// how fast the difference bar will go down
#define ENERGY_BAR_DIFF_SPEED 3.0

#define ENERGY_BAR_FRAME_START_X 32
#define ENERGY_BAR_FRAME_START_Y -16
#define ENERGY_BAR_FRAME_WIDTH (192 * 3)
#define ENERGY_BAR_FRAME_HEIGHT (20 * 3)

#define ENERGY_BAR_START_X (ENERGY_BAR_FRAME_START_X + 22 * 3)
#define ENERGY_BAR_START_Y (ENERGY_BAR_FRAME_START_Y + 6 * 3)
#define ENERGY_BAR_WIDTH (160 * 3)
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

#define SAY_OPEN_SPEED 800.0

int gui_font_id = 0;

// inventory counters
char metal_count_text[16] = "0";
char crystal_count_text[16] = "0";
char deuterium_count_text[16] = "0";

// energy bar
float playerHealth = 0;
float playerMaxHealth = 0;
u32 bar_frame_id = 0;
u32 bar_background_id = 0;
u32 bar_content_id = 0;
u32 bar_diff_id = 0;

// Pop up text
u32 pop_up_id = 0;
char *pop_up_text = NULL;
int pop_up_text_width = 0;

// Interaction message
u32 interact_button_id = 0;
u32 interact_background_id = 0;
char interact_text[32] = "E: Interact";
u32 interact_message_id = 0;

// say message
u32 say_background_id = 0;
u32 say_message_id = 0;
u32 say_background_arrow_id = 0;
char say_text[256] = "\0";
float say_timer = 0.0;
float say_width = 0.0;
float say_target_width = 0.0;

// Cursor
u32 cursor_id = 0;

void GUISetHide(int hide)
{
  CDrawable *drawable = CmpGetDrawable(bar_frame_id);
  drawable->visible = !hide;
  drawable = CmpGetDrawable(bar_background_id);
  drawable->visible = !hide;
  drawable = CmpGetDrawable(bar_content_id);
  drawable->visible = !hide;
  drawable = CmpGetDrawable(bar_diff_id);
  drawable->visible = !hide;
}

void GUI_show_pop_up(char *text, int x, int y)
{
  if (pop_up_text != text)
  {
    pop_up_text = text;
    pop_up_text_width = microFontGetTextWidth(gui_font_id, text);
  }

  x -= pop_up_text_width / 2;
  y += 128;

  // Update pop up
  if (pop_up_id != 0)
  {
    // Update visibility
    CDrawable *drawable = CmpGetDrawable(pop_up_id);
    drawable->visible = 1;

    // Update position
    CPosition *pos = CmpGetPosition(pop_up_id);

    pos->x = x;
    pos->y = y;

    // Update text
    CText *text_comp = CmpGetText(pop_up_id);
    text_comp->text = text;

    return;
  }

  // Create pop up
  pop_up_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(pop_up_id, x, y);
  CmpAddDrawable(pop_up_id, LAYER_UI_1, 1);
  CmpAddHud(pop_up_id);
  gui_font_id = microResourceGet("ui_font");
  CmpAddText(pop_up_id, gui_font_id, 3, TEXT_ALIGN_LEFT, text);
}

void GUI_hide_pop_up()
{
  if (pop_up_id == 0)
    return;
  CDrawable *drawable = CmpGetDrawable(pop_up_id);
  drawable->visible = 0;
}

void GUI_show_interact_message(char *message)
{
  if (interact_button_id != 0)
  {
    CDrawable *drawable = CmpGetDrawable(interact_button_id);
    drawable->visible = 1;
    drawable = CmpGetDrawable(interact_background_id);
    drawable->visible = 1;
    drawable = CmpGetDrawable(interact_message_id);
    drawable->visible = 1;
    CText *text = CmpGetText(interact_message_id);
    text->text = message;
    return;
  }
  float viewWidth, viewHeight;
  microViewGetViewport(&viewWidth, &viewHeight);

  // Background
  interact_background_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(interact_background_id, 8, viewHeight - 48 - 16);
  CmpAddDrawable(interact_background_id, LAYER_UI_1, 1);
  CmpAddHud(interact_background_id);
  const int texture_id = microResourceGet("atlas");
  MicroTextureSource txs = microTextureAtlasGetRegion(texture_id, "blank");
  CmpAddSprite(interact_background_id, texture_id, txs.x, txs.y, txs.w, txs.h);
  CmpAddTransform(interact_background_id, 256 - 8, 48 + 4, 0, 0, 0);
  CmpAddColor(interact_background_id, 0.0, 0.0, 0.0, 0.5);

  // Button
  interact_button_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(interact_button_id, 24 + 16, viewHeight - 24 - 16);
  CmpAddDrawable(interact_button_id, LAYER_UI_2, 1);
  CmpAddHud(interact_button_id);
  CmpAddSprite(interact_button_id, microResourceGet("atlas"), 0, 0, 16, 16);
  CmpAddTransform(interact_button_id, 48, 48, 24, 24, 0);
  const int button_anim_id = microAnimationGet("keyboard-button");
  CmpAddAnimation(interact_button_id, button_anim_id, 0.5, FALSE, FALSE, FALSE);

  // Message
  interact_message_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(interact_message_id, 24 + 8 + 48, viewHeight - 16 - 16);
  CmpAddDrawable(interact_message_id, LAYER_UI_2, 1);
  CmpAddHud(interact_message_id);
  CmpAddText(interact_message_id, gui_font_id, 3, TEXT_ALIGN_LEFT, message);
}

void GUI_hide_interact_message()
{
  if (interact_message_id == 0)
    return;
  CDrawable *drawable = CmpGetDrawable(interact_message_id);
  drawable->visible = 0;
  drawable = CmpGetDrawable(interact_button_id);
  drawable->visible = 0;
  drawable = CmpGetDrawable(interact_background_id);
  drawable->visible = 0;
}

void GUI_update_energy_bar(float dt)
{
  playerHealth = PlayerGetHealth();
  playerMaxHealth = PlayerGetMaxHealth();
  float normalized_energy = (float)playerHealth / (float)playerMaxHealth;

  // Update energy bar
  CTransform *bar_transform = CmpGetTransform(bar_content_id);
  bar_transform->width = (normalized_energy * ENERGY_BAR_WIDTH);

  CTransform *bar_diff_transform = CmpGetTransform(bar_diff_id);
  if (bar_transform->width > bar_diff_transform->width)
  {
    bar_diff_transform->width = bar_transform->width;
  }
  else if (bar_transform->width < bar_diff_transform->width)
  {
    float change = (bar_diff_transform->width - bar_transform->width) *
                   ENERGY_BAR_DIFF_SPEED * dt;
    if (change < 0.1)
      change = 0.1;
    bar_diff_transform->width -= change;
  }
}

void GUI_update_inventory()
{
  // Update inventory counters
  const int metal_count = inventoryGetCount(ITEM_METAL);
  const int crystal_count = inventoryGetCount(ITEM_CRYSTAL);
  const int deuterium_count = inventoryGetCount(ITEM_DEUTERIUM);
  sprintf(metal_count_text, "%d", metal_count);
  sprintf(crystal_count_text, "%d", crystal_count);
  sprintf(deuterium_count_text, "%d", deuterium_count);
}

void GUI_update_interact_message()
{
  const int interactable_eid = interactionSystemGetObjectId();
  if (interactable_eid == -1)
  {
    GUI_hide_interact_message();
    return;
  }
  else
  {
    CInteractive *interactive = (CInteractive *)
      microECSEntityGetComponent(interactable_eid, cid_interactive);
    if (interactive->interact_text == NULL)
      strcpy(interact_text, "E: Interact");
    else
    {
      strcpy(interact_text, "E: ");
      strcat(interact_text, interactive->interact_text);
    }
    GUI_show_interact_message(interact_text);
  }
}

void GUI_update_pop_up()
{
  const int interactable_eid = interactionSystemGetObjectId();
  if (interactable_eid == -1)
  {
    GUI_hide_pop_up();
    return;
  }
  else
  {
    CPosition *pos = (CPosition *)microECSEntityGetComponent(interactable_eid,
                                                             cid_position);
    CInteractive *interactive = (CInteractive *)
      microECSEntityGetComponent(interactable_eid, cid_interactive);

    float px, py;
    microViewPointWorldToScreen(pos->x, pos->y, &px, &py);

    if (interactive->interact_text != NULL)
      GUI_show_pop_up(interactive->interact_text, px, py);
    else
      GUI_show_pop_up("E: Interact", px, py);
  }
}

void GUI_update_player_say(float dt)
{
  if (say_timer <= 0.0)
  {
    CDrawable *drawable = CmpGetDrawable(say_message_id);
    drawable->visible = 0;

    say_width -= SAY_OPEN_SPEED * dt;
    if (say_width <= 0) {
      say_width = say_target_width;
    
      drawable = CmpGetDrawable(say_background_id);
      drawable->visible = 0;
      drawable = CmpGetDrawable(say_background_arrow_id);
      drawable->visible = 0;
    }
  }
  else
  {
    say_timer -= dt;
    say_width += SAY_OPEN_SPEED * dt;
    if (say_width >= say_target_width) {
      say_width = say_target_width;
      CDrawable *drawable = CmpGetDrawable(say_message_id);
      drawable->visible = 1;
    }
  }

  CTransform *transform = CmpGetTransform(say_background_id);
  transform->width = say_width;
  transform->originX = say_width / 2.0;
}

void GUI_update(int entity, float dt)
{
  (void)(entity); // Unused parameter
  
  PlayerState player_state = PlayerGetState();
  if (player_state == PLAYER_STATE_WARP_DRIVE ||
      player_state == PLAYER_STATE_LANDING ||
      player_state == PLAYER_STATE_DEPARTING)
    GUISetHide(TRUE);
  else
    GUISetHide(FALSE);

  GUI_update_energy_bar(dt);
  GUI_update_inventory();
  // GUI_update_pop_up();
  GUI_update_interact_message();
  GUI_update_player_say(dt);
}

u32 GUI_add_image(u32 atlasId, u32 x, u32 y,
                       char *image_name, u32 width, u32 height,
                       u32 origin_x, u32 origin_y, u32 layerId)
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
                               .layerId = layerId,
                               .visible = 1,
                             });
  microECSEntityAddComponent(entity_id, cid_hud, NULL);

  return entity_id;
}

u32 GUI_add_text(u32 x, u32 y, char *text)
{
  int entity_id = microECSEntityNew(NULL, NULL);
  microECSEntityAddComponent(entity_id, cid_position,
                             &(CPosition){.x = x, .y = y});

  microECSEntityAddComponent(entity_id, cid_drawable,
                             &(CDrawable){
                               .layerId = LAYER_UI_2,
                               .visible = 1,
                             });
  microECSEntityAddComponent(entity_id, cid_hud, NULL);

  u32 font_id = microResourceGet("ui_font");

  microECSEntityAddComponent(entity_id, cid_text,
                             &(CText){
                               .text = text,
                               .lineSpacing = 3,
                               .fontId = font_id,
                             });

  return entity_id;
}

void GUI_free(int entityId)
{
  (void)(entityId); // Unused parameter
}

void GUI_update_cursor(int entityId, float dt)
{
  (void)(dt); // Unused parameter

  int mouse_x, mouse_y;
  microSystemGetMousePos(&mouse_x, &mouse_y);
  CPosition *pos = CmpGetPosition(entityId);
  pos->x = mouse_x;
  pos->y = mouse_y;
}

void GUIInit()
{
  // Hide cursor
  microSystemShowCursor(0);

  // Create handler for updating life UI and freeing memory
  int gui_handler = microECSEntityNew(NULL, GUI_free);
  microECSEntityAddComponent(gui_handler, cid_update,
                             &(CUpdate){.update = GUI_update});

  // Creating and computing lifes
  playerHealth = PlayerGetHealth();
  playerMaxHealth = PlayerGetMaxHealth();

  const int atlasId = microResourceGet("atlas");

  int window_width, window_height;
  microSystemGetWindowSize(&window_width, &window_height);

  // Add energy bar
  bar_background_id = GUI_add_image(atlasId,
                                    window_width / 2 -
                                      ENERGY_BAR_FRAME_WIDTH / 2 +
                                      ENERGY_BAR_START_X,
                                    ENERGY_BAR_START_Y + window_height -
                                      ENERGY_BAR_FRAME_HEIGHT,
                                    "energy_bar_background", ENERGY_BAR_WIDTH,
                                    ENERGY_BAR_HEIGHT, 0, 0, 5);

  bar_diff_id = GUI_add_image(atlasId,
                              window_width / 2 - ENERGY_BAR_FRAME_WIDTH / 2 +
                                ENERGY_BAR_START_X,
                              ENERGY_BAR_START_Y + window_height -
                                ENERGY_BAR_FRAME_HEIGHT,
                              "energy_bar_content", ENERGY_BAR_WIDTH,
                              ENERGY_BAR_HEIGHT, 0, 0, 6);

  microECSEntityAddComponent(bar_diff_id, cid_color,
                             &(CColor){
                               .r = 1.0,
                               .g = 0.0,
                               .b = 0.0,
                               .a = 1.0,
                             });

  bar_content_id = GUI_add_image(atlasId,
                                 window_width / 2 - ENERGY_BAR_FRAME_WIDTH / 2 +
                                   ENERGY_BAR_START_X,
                                 ENERGY_BAR_START_Y + window_height -
                                   ENERGY_BAR_FRAME_HEIGHT,
                                 "energy_bar_content", ENERGY_BAR_WIDTH,
                                 ENERGY_BAR_HEIGHT, 0, 0, 7);

  bar_frame_id = GUI_add_image(atlasId,
                               window_width / 2 - ENERGY_BAR_FRAME_WIDTH / 2 +
                                 ENERGY_BAR_FRAME_START_X,
                               ENERGY_BAR_FRAME_START_Y + window_height -
                                 ENERGY_BAR_FRAME_HEIGHT,
                               "energy_bar_frame", ENERGY_BAR_FRAME_WIDTH,
                               ENERGY_BAR_FRAME_HEIGHT, 0, 0, 8);

  // // Add metal counter
  // GUI_add_image(atlasId, RESOURCES_START_X, RESOURCES_START_Y, "UI_counter",
  //               RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0, 0);
  //
  // GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
  //               RESOURCES_START_Y + RESOURCES_ICON_OFFSET, "metal",
  //               RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);
  //
  // GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
  //              RESOURCES_START_Y + RESOURCES_TEXT_VOFFSET, metal_count_text);
  //
  // // Add crystal counter
  // GUI_add_image(atlasId, RESOURCES_START_X,
  //               RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT + RESOURCES_SPACING,
  //               "UI_counter", RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0,
  //               0);
  //
  // GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
  //               RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 1 +
  //                 RESOURCES_SPACING + RESOURCES_ICON_OFFSET,
  //               "crystal", RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);
  //
  // GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
  //              RESOURCES_START_Y + RESOURCES_SPACING +
  //                RESOURCES_FRAME_HEIGHT * 1 + RESOURCES_TEXT_VOFFSET,
  //              crystal_count_text);
  //
  // // Add deuterium counter
  // GUI_add_image(atlasId, RESOURCES_START_X,
  //               RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 2 +
  //                 RESOURCES_SPACING * 2,
  //               "UI_counter", RESOURCES_FRAME_WIDTH, RESOURCES_FRAME_HEIGHT, 0,
  //               0);
  //
  // GUI_add_image(atlasId, RESOURCES_START_X + RESOURCES_ICON_OFFSET,
  //               RESOURCES_START_Y + RESOURCES_FRAME_HEIGHT * 2 +
  //                 RESOURCES_SPACING * 2 + RESOURCES_ICON_OFFSET,
  //               "deuterium", RESOURCES_ICON_SIZE, RESOURCES_ICON_SIZE, 0, 0);
  //
  // GUI_add_text(RESOURCES_START_X + RESOURCES_TEXT_HOFFSET,
  //              RESOURCES_START_Y + RESOURCES_SPACING * 2 +
  //                RESOURCES_FRAME_HEIGHT * 2 + RESOURCES_TEXT_VOFFSET,
  //              deuterium_count_text);

  // Add cursor
  cursor_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(cursor_id, 0, 0);
  CmpAddDrawable(cursor_id, LAYER_UI_1, TRUE);
  CmpAddHud(cursor_id);
  MicroTextureSource txs = microTextureAtlasGetRegion(atlasId, "cursor");
  CmpAddSprite(cursor_id, atlasId, txs.x, txs.y, txs.w, txs.h);
  CmpAddTransform(cursor_id, 24, 24, 12, 12, 0);
  CmpAddUpdate(cursor_id, GUI_update_cursor);
  
  // === Say message ===
  say_timer = 0.0;

  // Add say background
  float viewWidth, viewHeight;
  microViewGetViewport(&viewWidth, &viewHeight);
  say_background_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(say_background_id, viewWidth / 2.0, viewHeight / 2.0 - 94);
  CmpAddDrawable(say_background_id, LAYER_UI_1, TRUE);
  CmpAddHud(say_background_id);
  txs = microTextureAtlasGetRegion(atlasId, "blank");
  CmpAddSprite(say_background_id, atlasId, txs.x, txs.y, txs.w, txs.h);
  CmpAddTransform(say_background_id, 256, 128, 128, 64, 0);
  CmpAddColor(say_background_id, 0.0, 0.0, 0.0, 0.7);

  // Add say message
  say_message_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(say_message_id, viewWidth / 2.0, viewHeight / 2.0 - 128);
  CmpAddDrawable(say_message_id, LAYER_UI_1, TRUE);
  CmpAddHud(say_message_id);
  CmpAddText(say_message_id, gui_font_id, 3, TEXT_ALIGN_LEFT, say_text);
  
  // Add say background arrow
  say_background_arrow_id = microECSEntityNew(NULL, NULL);
  CmpAddPosition(say_background_arrow_id, viewWidth / 2.0,
                 viewHeight - 94 - 16 + 128);
  CmpAddDrawable(say_background_arrow_id, LAYER_UI_1, TRUE);
  CmpAddHud(say_background_arrow_id);
  txs = microTextureAtlasGetRegion(atlasId, "triangle");
  CmpAddSprite(say_background_arrow_id, atlasId, txs.x, txs.y + txs.h, txs.w, -txs.h);
  CmpAddTransform(say_background_arrow_id, 32, 16, 16, 8, 0);
  CmpAddColor(say_background_arrow_id, 0.0, 0.0, 0.0, 0.7);
}

void GUIPlayerSay(const char *text)
{
  strcpy(say_text, text);
  const u32 text_len_px = microFontGetTextWidth(gui_font_id, say_text);

  u32 lines = 1;
  for (int i = 0; i < (int)strlen(say_text); i++)
    if (say_text[i] == '\n')
      lines++;
  
  // Draw say message
  CDrawable *drawable = CmpGetDrawable(say_background_id);
  drawable->visible = 1;
  drawable = CmpGetDrawable(say_message_id);
  drawable->visible = 0;
  drawable = CmpGetDrawable(say_background_arrow_id);
  drawable->visible = 1;
  say_timer = strlen(say_text) * 0.2;
  
  // Update position and size of say message
  float viewWidth, viewHeight;
  microViewGetViewport(&viewWidth, &viewHeight);

  CPosition *pos = CmpGetPosition(say_background_id);
  pos->y = viewHeight / 2.0 - 96;
  CTransform *transform = CmpGetTransform(say_background_id);
  transform->width = text_len_px + 32;
  transform->height = 24 * lines + 8;
  transform->originX = transform->width / 2.0;
  transform->originY = transform->height / 2.0;

  CPosition *arrow_pos = CmpGetPosition(say_background_arrow_id);
  arrow_pos->y = viewHeight/2.0 - 94 + transform->height / 2.0 + 4;

  pos = CmpGetPosition(say_message_id);
  pos->x = viewWidth/2.0 - text_len_px/2.0;
  pos->y = viewHeight / 2.0 - 94 - 24 * lines / 2.0 + 4 + 24.0/2;

  say_width = 0.0;
  say_target_width = transform->width;
}

