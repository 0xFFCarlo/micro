#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../components/LogicComponents.h"
#include "../micro/Resources.h"
#include "../micro/Graphics.h"
#include "../micro/ECS.h"
#include "../micro/Physics.h"
#include <stdio.h>
#include <assert.h>
#include "../util/perlin_noise.h"
#include "Cave.h"

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
float planetLightDepth = 120;

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
  
  CShadedCanvas* canvas = microECSEntityGetComponent(shadow_id, cid_shadedCanvas);
  canvas->needsUpdate = 1;
}

unsigned char* makePlanetTexture(int width, int height)
{
  unsigned char* texture = (unsigned char*)malloc(width * height * 4);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      float distX = (float)x - (float)width / 2.0;
      float distY = (float)y - (float)height / 2.0;
      float distNorm = sqrt(distX * distX + distY * distY) / ((float)width * 0.5);
      // distNorm += noise2((float)x / 100.0, (float)y / 100.0) * 0.04;

      //Compute color
      float f = 20.0;
      float r = (noise2((float)x / f, (float)y / f) + 1.0) * 0.5;
      float r2 = ((double)rand() / (double)RAND_MAX -0.5) * 2.0;
      int r_quant = (0.3 + r * 0.2 + r2 * 0.15) * 255;
      // int r_q_reduced = floor((float)r_quant / 3) * 3;
      int r_q_reduced = r_quant;
      texture[(y * width + x) * 4 + 0] = r_q_reduced;
      texture[(y * width + x) * 4 + 1] = r_q_reduced;
      texture[(y * width + x) * 4 + 2] = r_q_reduced;
      
      //Compute alpha
      texture[(y * width + x) * 4 + 3] = 255 * (distNorm < 1.0);
    }
  }

  return texture;
}

void setupPlanetEntity(int planetId)
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  float canvas_planet_height = planetRadius / 2.0;
  float canvas_planet_width = planetRadius / 2.0;
  float planet_scale = planetScaleFactor;

  // Load shader and apply parameters
  // shader_id = microShaderLoadFromFile("./res/shaders/base_vert.glsl", "./res/shaders/planet.glsl");
  // assert(shader_id != -1);
  // int current_shader = microShaderGetCurrent();
  // microShaderApply(shader_id);
  // microViewApply(); // send view matrix to shader
  // microShaderSetUniform("resolution", canvas_planet_width, canvas_planet_height);
  // microShaderApply(current_shader);
  
  // Position component
  planetX = viewWidth / 2.f;
  planetY = viewHeight / 2.f + (float)canvas_planet_height + (float)viewHeight * 0.2 / 2.0;
  microECSEntityAddComponent(planetId, cid_position, &(CPosition) {
      .x = planetX,
      .y = planetY,
      });
  
  // Shaded canvas component
  // planet_canvas_id = microCanvasCreate(canvas_planet_width, canvas_planet_height);
  // microECSEntityAddComponent(planetId, cid_shadedCanvas, &(CShadedCanvas) {
  //     .canvasId = planet_canvas_id,
  //     .shaderId = shader_id,
  //     .width = canvas_planet_width,
  //     .height = canvas_planet_height,
  //     .needsUpdate = 1,
  //     });

  // Sprite component
  // int textureId = microCanvasGetTextureId(planet_canvas_id);
  unsigned char* texture = makePlanetTexture(canvas_planet_width, canvas_planet_height);
  int textureId = microTextureLoadFromMemory(texture, canvas_planet_width, canvas_planet_height, 4, MICRO_FILTER_NEAREST);
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
  microECSEntityAddComponent(planetId, cid_drawable, &(CDrawable){ .layerId = 1, .visible = 1});
  microECSEntityAddComponent(planetId, cid_update, &(CUpdate){ .update = planetUpdate });
  
  // Add caves
  CaveAddEntity(planetX, planetY - PlanetGetRadius() / 2.0 + 32.0 / 2 - 4);
}

void setupShadow(int shadowId)
{
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  float canvas_posteffect_height = 512.0;
  float canvas_posteffect_width = viewWidth * (canvas_posteffect_height / viewHeight);

  // Load shader and apply parameters
  shadow_shader_id = microShaderLoadFromFile("./res/shaders/base_vert.glsl", "./res/shaders/shadow.glsl");
  assert(shadow_shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(shadow_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_posteffect_width, canvas_posteffect_height);
  microShaderSetUniform("radius", PlanetGetRadius() / viewHeight);
  microShaderSetUniform("lightDepth", planetLightDepth / viewHeight);
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
      .needsUpdate = 1,
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
  float viewportWidth, viewportHeight;
  microViewGetViewport(&viewportWidth, &viewportHeight);
  microECSEntityAddComponent(shadowId, cid_transform, &(CTransform) {
      .width = viewportWidth,
      .height = viewportHeight,
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
  microECSEntityAddComponent(shadowId, cid_drawable, &(CDrawable){.layerId = 2, .visible = 1});
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
