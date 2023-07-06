#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "Planet.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

int spaceId = -1;
int space_shader_id = -1;
int space_canvas_id = -1;
double curr_time = 0.0;


// Updates shader parameters to show the planet atmosphere
// in the right place
void spaceUpdate(int spaceId, float dt)
{
  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);

  float viewX, viewY;
  float viewWidth, viewHeight;
  float viewAngle;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);
  viewAngle = microViewGetRotation() * 3.14159265358979323846 / 180.0;

  float windowWidth, windowHeight;
  microViewGetSize(&windowWidth, &windowHeight);
  
  float normPlanetX = ((planetX - viewX) * 2.0) / windowHeight;
  float normPlanetY = ((planetY - viewY) * 2.0) / windowHeight;

  curr_time += dt * 0.03;
  
  microShaderApply(space_shader_id);
  microShaderSetUniform("planet_center", normPlanetX, normPlanetY);
  //microShaderSetUniform("view_center", 0, dist);
  microShaderSetUniform("view_angle", viewAngle);
  microShaderSetUniform("time", curr_time);
  microShaderApply(0);

  CShadedCanvas* canvas = microECSEntityGetComponent(spaceId, cid_shadedCanvas);
  canvas->needsUpdate = 1;
}

void SpaceEntityAdd()
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);
  float viewX, viewY;
  microViewGetCenter(&viewX, &viewY);

  // Create planet
  spaceId = microECSEntityNew(NULL, NULL);
  float canvas_space_height = 512;
  float canvas_space_width = viewWidth * (canvas_space_height / viewHeight);
  float planet_radius = PlanetGetRadius() / viewHeight;
  float view_angle = 0.0;
  float nebulaColor[3] = {0.57, 0.27, 0.41};
  float atmosphereMaxIntensity = 0.5;
  float atmosphereDecay = 0.9;
  float atmosphereColor[4] = {0.6, 0.6, 1.0, 1.0};
  float starfieldThreshold1 = 40.0;
  float starfieldThreshold2 = 40.0;

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);

  // Load shader and apply parameters
  space_shader_id = microShaderLoadFromFile("./res/shaders/base_vert.glsl", "./res/shaders/space.glsl");
  assert(space_shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(space_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_space_width, canvas_space_height);
  microShaderSetUniform("time", curr_time);
  microShaderSetUniform("view_center", 0.0, (viewY - planetY) * 1.8 / viewHeight);
  microShaderSetUniform("view_angle", view_angle);
  microShaderSetUniform("planet_radius", planet_radius);
  microShaderSetUniform("planet_center", 0.0, (viewY - planetY) * 1.8 / viewHeight);
  microShaderSetUniform("nebulaColor", nebulaColor[0], nebulaColor[1], nebulaColor[2]);
  microShaderSetUniform("starfieldThreshold1", starfieldThreshold1);
  microShaderSetUniform("starfieldThreshold2", starfieldThreshold2);
  microShaderSetUniform("atmosphereMaxIntensity", atmosphereMaxIntensity);
  microShaderSetUniform("atmosphereDecay", atmosphereDecay);
  microShaderSetUniform("atmosphereColor", atmosphereColor[0], atmosphereColor[1], atmosphereColor[2], atmosphereColor[3]);

  microShaderApply(current_shader);
  
  // Position component
  microECSEntityAddComponent(spaceId, cid_position, &(CPosition){ 
      .x = 0,
      .y = 0,
      });

  // Shaded canvas component
  int canvasId = microCanvasCreate(canvas_space_width, canvas_space_height);
  microECSEntityAddComponent(spaceId, cid_shadedCanvas, &(CShadedCanvas){ 
      .canvasId = canvasId,
      .shaderId = space_shader_id,
      .width = canvas_space_width,
      .height = canvas_space_height,
      .needsUpdate = 1,
      });

  // Sprite component
  int textureId = microCanvasGetTextureId(canvasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  microECSEntityAddComponent(spaceId, cid_sprite, &(CSprite){ 
      .textureId = textureId,
      .tx = 0,
      .ty = 0,
      .tw = texWidth,
      .th = texHeight,
      });
  
  // Transform component
  float viewportWidth, viewportHeight;
  microViewGetViewport(&viewportWidth, &viewportHeight);
  microECSEntityAddComponent(spaceId, cid_transform, &(CTransform){ 
      .width = viewportWidth,
      .height = viewportHeight,
      .originX = 0,
      .originY = 0,
      .rotation = 0,
      });

  // Color component
  microECSEntityAddComponent(spaceId, cid_color, &(CColor){ 
      .r = 1.0,
      .g = 1.0,
      .b = 1.0,
      .a = 1.0 
      });
  microECSEntityAddComponent(spaceId, cid_drawable, &(CDrawable){.layerId = 0, .visible = 1});
  microECSEntityAddComponent(spaceId, cid_hud, &(CHud){});
  microECSEntityAddComponent(spaceId, cid_update, &(CUpdate){.update = spaceUpdate});
}
