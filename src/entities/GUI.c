#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include "../micro/State.h"
#include "../micro/Vector.h"
#include "Player.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define HEARTH_WIDTH 32
#define HEARTH_HEIGHT 32
#define HEARTH_SPACING 10
#define HEARTH_MARGIN 32

Vector lifeSpritesEntityIds;
int playerHealth = 0;
int playerMaxHealth = 0;
int hearthsCount = 0;
int hearthsMaxCount = 0;

void GUIFree()
{
  vector_free(&lifeSpritesEntityIds);
}

void GUIInit()
{
  lifeSpritesEntityIds = vector_create(sizeof(int)); 

  playerHealth = PlayerGetHealth();
  playerMaxHealth = PlayerGetMaxHealth();
  hearthsMaxCount = playerMaxHealth / 4;
  hearthsCount = playerHealth / 4;

  // Create hearths
  int tmp = playerHealth;
  for (int i = 0; i < hearthsMaxCount; i++) {
    
    void(*freeFunc)(void*) = NULL;
    if (i == 0) freeFunc = GUIFree;
    int hearth_id = microECSEntityNew(NULL, freeFunc);
    vector_push_back(&lifeSpritesEntityIds, &hearth_id);
  
    const int atlasId = microResourceGet("atlas");
    const int textureId = microTextureAtlasGetTextureId(atlasId);
    const MicroTextureSource ts = microTextureAtlasGetRegion(atlasId, "life-4-4");
  
    microECSEntityAddComponent(hearth_id, cid_position, &(CPosition){
        .x = HEARTH_WIDTH * i + HEARTH_WIDTH/2.0 + HEARTH_MARGIN + HEARTH_SPACING * i,
        .y = HEARTH_HEIGHT/2.0 + HEARTH_MARGIN,
        });

    microECSEntityAddComponent(hearth_id, cid_sprite, &(CSprite){
        .textureId = textureId,
        .tx = ts.x,
        .ty = ts.y,
        .tw = ts.w,
        .th = ts.h,
        });

    // Transform component
    microECSEntityAddComponent(hearth_id, cid_transform, &(CTransform){
        .width = HEARTH_WIDTH,
        .height = HEARTH_HEIGHT,
        .originX = HEARTH_WIDTH/2.0,
        .originY = HEARTH_HEIGHT/2.0,
        .rotation = 0.0,
        });

    microECSEntityAddComponent(hearth_id, cid_drawable, &(CDrawable){
        .layerId = 5,
        .visible = 1,
        });
    microECSEntityAddComponent(hearth_id, cid_hud, NULL);

    // microECSEntityAddComponent(log_gui_entity_id, cid_update, &(CUpdate){
    //     .update = updateLogGUI
    //     });

  }
}
