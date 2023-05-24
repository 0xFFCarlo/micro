#include <stdlib.h>
#include "RenderingComponents.h"
#include "../ECS.h"

int cid_sprite = -1;
int cid_text = -1;
int cid_color = -1;
int cid_layer = -1;
int cid_hud = -1;
int cid_animation = -1;
int cid_shadedCanvas = -1;
int cid_lock_on_view = -1;


void RegisterCSprite()
{
  cid_sprite = microECSComponentRegister(sizeof(CSprite), NULL);
}

void RegisterCText()
{
  cid_text = microECSComponentRegister(sizeof(CText), NULL);
}

void RegisterCColor()
{
  cid_color = microECSComponentRegister(sizeof(CColor), NULL);
}

void RegisterCLayer()
{
  cid_layer = microECSComponentRegister(sizeof(CLayer), NULL);
}

void RegisterCHud()
{
  cid_hud = microECSComponentRegister(sizeof(CHud), NULL);
}

void RegisterCAnimation() {
  cid_animation = microECSComponentRegister(sizeof(CAnimation), NULL);
}

void RegisterCShadedCanvas()
{
  cid_shadedCanvas = microECSComponentRegister(sizeof(CShadedCanvas), NULL);
}

void RegisterCLockOnView()
{
  cid_lock_on_view = microECSComponentRegister(sizeof(CLockOnView), NULL);
}
