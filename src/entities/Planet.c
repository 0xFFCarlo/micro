#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include <stdio.h>
#include <assert.h>

#define LIGHT_DEPTH 0.2

int planet_id = -1;
int planet_shader_id = -1;
int planet_canvas_id = -1;
int shadow_id = -1;
int shadow_shader_id = -1;
int shadow_canvas_id = -1;

float planetX = 0.0;
float planetY = 0.0;
float planetRadius = 800;
float planetScaleFactor = 2.0;

int planet_body_id = -1;

float PlanetGetRadius()
{
  return planetRadius * planetScaleFactor / 2.0;
}

void PlanetGetPos(float* x, float* y)
{
  *x = planetX;
  *y = planetY;
}


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

  // float normViewX = viewX / windowHeight;
  // float normViewY = -viewY / windowHeight;

  microShaderApply(shadow_shader_id);
  microShaderSetUniform("planet_center", normPlanetX, normPlanetY);
  //microShaderSetUniform("view_center", normViewX, normViewY);
  microShaderSetUniform("view_angle", viewAngle);
  microShaderApply(0);
}

void setupPlanetEntity(int planetId)
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  int shader_id;
  float canvas_planet_height = planetRadius / 2.0;
  float canvas_planet_width = planetRadius / 2.0;
  float planet_scale = planetScaleFactor;

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
  microECSEntityAddComponent(planetId, cid_position, &(CPosition) {
      .x = planetX,
      .y = planetY,
      });

  // Shaded canvas component
  planet_canvas_id = microCanvasCreate(canvas_planet_width, canvas_planet_height);
  microECSEntityAddComponent(planetId, cid_shadedCanvas, &(CShadedCanvas) {
      .canvasId = planet_canvas_id,
      .shaderId = shader_id,
      .width = canvas_planet_width,
      .height = canvas_planet_height,
      });

  // Sprite component
  int textureId = microCanvasGetTextureId(planet_canvas_id);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  microECSEntityAddComponent(planetId, cid_sprite, &(CSprite) {
      .textureId = textureId,
      .tx = 0,
      .ty = 0,
      .tw = texWidth,
      .th = texHeight,
      });

  // Transform component
  microECSEntityAddComponent(planetId, cid_transform, &(CTransform) {
      .width = canvas_planet_width * planet_scale,
      .height = canvas_planet_height * planet_scale,
      .originX = canvas_planet_width * planet_scale / 2.0,
      .originY = canvas_planet_height * planet_scale / 2.0,
      .rotation = 0,
      });
  
  // Body component
  planet_body_id = microPhysicsBodyNewCircle(0, planetX, planetY, PlanetGetRadius()/2.0 - 10, 1.0, 1, 1.0, 0.0, 1.0);
  microECSEntityAddComponent(planetId, cid_body, &(CBody) {
      .body_id = planet_body_id,
      });

  // Color component
  microECSEntityAddComponent(planetId, cid_color, &(CColor){
      .r = 1.0,
      .g = 1.0,
      .b = 1.0,
      .a = 1.0 
      });
  microECSEntityAddComponent(planetId, cid_drawable, &(CDrawable){ .layerId = 1 });
  microECSEntityAddComponent(planetId, cid_update, &(CUpdate){ .update = planetUpdate });
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
  microShaderSetUniform("radius", PlanetGetRadius() / viewHeight);
  microShaderSetUniform("lightDepth", LIGHT_DEPTH);
  microShaderSetUniform("planet_center", 0.0, 0.2 + PlanetGetRadius() / viewHeight);
  microShaderSetUniform("view_angle", 0.0);
  microShaderApply(current_shader);

  // Position component
  microECSEntityAddComponent(shadowId, cid_position, &(CPosition) {
      .x = 0,
      .y = 0,
      });

  // Shaded canvas component
  shadow_canvas_id = microCanvasCreate(canvas_posteffect_width, canvas_posteffect_height);
  microECSEntityAddComponent(shadowId, cid_shadedCanvas, &(CShadedCanvas) {
      .canvasId = shadow_canvas_id,
      .shaderId = shadow_shader_id,
      .width = canvas_posteffect_width,
      .height = canvas_posteffect_height,
      });

  // Sprite component
  int textureId = microCanvasGetTextureId(shadow_canvas_id);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  microECSEntityAddComponent(shadowId, cid_sprite, &(CSprite) {
      .textureId = textureId,
      .tx = 0,
      .ty = 0,
      .tw = texWidth,
      .th = texHeight,
      });
  microECSEntityAddComponent(shadowId, cid_transform, &(CTransform) {
      .width = canvas_posteffect_width * posteffect_scale,
      .height = canvas_posteffect_height * posteffect_scale,
      .originX = 0,
      .originY = 0,
      .rotation = 0,
      });
  microECSEntityAddComponent(shadowId, cid_color, &(CColor) {
      .r = 1.0,
      .g = 1.0,
      .b = 1.0,
      .a = 1.0
      });
  microECSEntityAddComponent(shadowId, cid_drawable, &(CDrawable){.layerId = 2});
  microECSEntityAddComponent(shadowId, cid_hud, &(CHud){});
}

void PlanetEntityAdd()
{

  // Create planet
  planet_id = microECSEntityNew(NULL, NULL);
  setupPlanetEntity(planet_id);

  // Create atmosphere and shadow
  shadow_id = microECSEntityNew(NULL, NULL);
  setupShadow(shadow_id);
}
