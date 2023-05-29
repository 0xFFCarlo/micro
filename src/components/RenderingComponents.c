#include <stdlib.h>
#include "RenderingComponents.h"
#include "../ECS.h"

int cid_sprite = -1;
int cid_text = -1;
int cid_color = -1;
int cid_drawable = -1;
int cid_hud = -1;
int cid_animation = -1;
int cid_shadedCanvas = -1;
int cid_lock_on_view = -1;


void RegisterCSprite()
{
  cid_sprite = microECSComponentRegister(sizeof(CSprite));
}

void RegisterCText()
{
  cid_text = microECSComponentRegister(sizeof(CText));
}

void RegisterCColor()
{
  cid_color = microECSComponentRegister(sizeof(CColor));
}

void RegisterCDrawable()
{
  cid_drawable = microECSComponentRegister(sizeof(CDrawable));
}

void RegisterCHud()
{
  cid_hud = microECSComponentRegister(sizeof(CHud));
}

void RegisterCAnimation() {
  cid_animation = microECSComponentRegister(sizeof(CAnimation));
}

void RegisterCShadedCanvas()
{
  cid_shadedCanvas = microECSComponentRegister(sizeof(CShadedCanvas));
}

void RegisterCLockOnView()
{
  cid_lock_on_view = microECSComponentRegister(sizeof(CLockOnView));
}

void RegisterRenderingComponents()
{
  RegisterCSprite();
  RegisterCText();
  RegisterCColor();
  RegisterCDrawable();
  RegisterCHud();
  RegisterCAnimation();
  RegisterCShadedCanvas();
  RegisterCLockOnView();
}
