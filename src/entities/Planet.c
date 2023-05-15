#include "../components/CPosition.h"
#include "../components/CSprite.h"
#include "../components/CShadedCanvas.h"
#include "../components/CUpdate.h"
#include "../Resources.h"
#include "../Graphics.h"
#include "../ECS.h"
#include <stdio.h>
#include <assert.h>

#define PLANET_RADIUS 1.0
#define LIGHT_DEPTH 0.2

int planet_id = -1;
int planet_shader_id = -1;
int planet_canvas_id = -1;
int shadow_id = -1;
int shadow_shader_id = -1;
int shadow_canvas_id = -1;

float planetX = 0.0;
float planetY = 0.0;
float planetRadius = PLANET_RADIUS;


// Updates shader parameters to show the shadows at the planet position
void planetUpdate(int planetId, float dt)
{
  CPosition* position = (CPosition*)microECSEntityGetComponent(planetId, cid_position);  
  
  float viewX, viewY;
  float viewWidth, viewHeight;
  float viewAngle;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);
  viewAngle = microViewGetRotation() * 3.14159265358979323846 / 180.0;
  
  float windowWidth, windowHeight;
  microViewGetSize(&windowWidth, &windowHeight);

  float normPlanetX = ((position->x - viewX) * 2.0) / windowHeight;
  float normPlanetY = ((position->y - viewY) * 2.0) / windowHeight;
  
  float normViewX = viewX / windowHeight;
  float normViewY = -viewY / windowHeight;
  
  microShaderApply(shadow_shader_id);
  microShaderSetUniform("planet_center", normPlanetX, normPlanetY);
  microShaderSetUniform("view_center", normViewX, normViewY);
  microShaderSetUniform("view_angle", viewAngle);
  microShaderApply(0);
}

void setupPlanetEntity(int planetId)
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  int shader_id;
  float ratio = 0.5859; //viewHeight / viewWidth;
  float canvas_planet_height = (512.0 * ratio) * PLANET_RADIUS;
  float canvas_planet_width = (512.0 * ratio) * PLANET_RADIUS;
  float planet_scale = 2.0;

  // Load shader and apply parameters
  shader_id = microShaderLoadFromFile("./res/shaders/base_vert.glsl", "./res/shaders/planet.glsl");
  assert(shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_planet_width, canvas_planet_height);
  microShaderApply(current_shader);
  
  // Position component
  planetX = viewWidth / 2.f;
  planetY = viewHeight / 2.f + (float)canvas_planet_height + (float)viewHeight * 0.2 / 2.0;
  CPosition position = {
    .x = planetX,
    .y = planetY,
  };
  microECSEntityAddComponent(planetId, cid_position, &position);
  

  // Shaded canvas component
  planet_canvas_id = microCanvasCreate(canvas_planet_width, canvas_planet_height);
  CShadedCanvas scanvas = {
    .canvasId = planet_canvas_id,
    .shaderId = shader_id,
    .width = canvas_planet_width,
    .height = canvas_planet_height,
  };
  microECSEntityAddComponent(planetId, cid_shadedCanvas, &scanvas);
 

  // Sprite component
  int textureId = microCanvasGetTextureId(scanvas.canvasId);
  microTexttureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  CSprite sprite = {
    .textureId = textureId,
    .tx = 0,
    .ty = 0,
    .tw = texWidth,
    .th = texHeight,
    .width = canvas_planet_width * planet_scale,
    .height = canvas_planet_height * planet_scale,
    .originX = canvas_planet_width,
    .originY = canvas_planet_height,
    .rotation = 0,
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0,
    .layerId = 1,
    .hud = 0,
  };
  microECSEntityAddComponent(planetId, cid_sprite, &sprite);

  // Update component
  CUpdate update = {
    .update = planetUpdate,
  };
  microECSEntityAddComponent(planetId, cid_update, &update);
}

void setupShadow(int shadowId)
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  float canvas_posteffect_height = 512.0;
  float canvas_posteffect_width = viewWidth * (canvas_posteffect_height / viewHeight);
  float posteffect_scale = viewWidth / canvas_posteffect_width;
  
  // Load shader and apply parameters
  shadow_shader_id = microShaderLoadFromFile("./res/shaders/base_vert.glsl", "./res/shaders/shadow.glsl");
  assert(shadow_shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(shadow_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_posteffect_width, canvas_posteffect_height);
  microShaderSetUniform("radius", PLANET_RADIUS);
  microShaderSetUniform("lightDepth", LIGHT_DEPTH);
  microShaderSetUniform("planet_center", 0.0, 0.2 + PLANET_RADIUS);
  microShaderSetUniform("view_center", 0.0, 0.2 + PLANET_RADIUS);
  microShaderSetUniform("view_angle", 0.0);
  microShaderApply(current_shader);

  // Position component
  CPosition position = {
    .x = 0,
    .y = 0,
  };
  microECSEntityAddComponent(shadowId, cid_position, &position);

  // Shaded canvas component
  shadow_canvas_id = microCanvasCreate(canvas_posteffect_width, canvas_posteffect_height);
  CShadedCanvas scanvas = {
    .canvasId = shadow_canvas_id,
    .shaderId = shadow_shader_id,
    .width = canvas_posteffect_width,
    .height = canvas_posteffect_height,
  };
  microECSEntityAddComponent(shadowId, cid_shadedCanvas, &scanvas);

  // Sprite component
  int textureId = microCanvasGetTextureId(scanvas.canvasId);
  microTexttureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  CSprite sprite = {
    .textureId = textureId,
    .tx = 0,
    .ty = 0,
    .tw = texWidth,
    .th = texHeight,
    .width = canvas_posteffect_width * posteffect_scale,
    .height = canvas_posteffect_height * posteffect_scale,
    .originX = 0,
    .originY = 0,
    .rotation = 0,
    .r = 1.0,
    .g = 1.0,
    .b = 1.0,
    .a = 1.0,
    .layerId = 2,
    .hud = 1,
  };
  microECSEntityAddComponent(shadowId, cid_sprite, &sprite);
}

void PlanetEntityAdd()
{

  // Create planet
  planet_id = microECSEntityNew(NULL, NULL, NULL);
  setupPlanetEntity(planet_id);

  // Create atmosphere and shadow
  shadow_id = microECSEntityNew(NULL, NULL, NULL);
  setupShadow(shadow_id);
}

int PlanetGetRadius()
{
  return planetRadius;
}

void PlanetGetPos(float* x, float* y)
{
  *x = planetX;
  *y = planetY;
}
