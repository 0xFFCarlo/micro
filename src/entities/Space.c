#include "../micro/components/LogicComponents.h"
#include "../micro/components/MotionComponents.h"
#include "../micro/components/RenderingComponents.h"
#include "../micro/core/ECS.h"
#include "../micro/core/Graphics.h"
#include "../micro/core/Resources.h"
#include "../micro/util/debug.h"
#include "../misc/layers.h"
#include "Planet.h"
#include "Player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

// Controls the resolution of the space shader
#define SPACE_CANVAS_HEIGHT 512

int spaceId = -1;
int space_shader_id = -1;
int space_canvas_id = -1;

// Space state
float rotation_angle = 0.0;
double warp_position = 0.0;
double warp_speed = 0.0;
double warp_acceleration = 0.01;
uint8_t warp_enabled = 0;

// Space look settings
float nebulaColor[3] = {0.57, 0.27, 0.41};
float atmosphereMaxIntensity = 0.5;
float atmosphereDecay = 0.90;
float atmosphereColor[4] = {0.6, 0.6, 1.0, 1.0};
float starfieldThreshold1 = 40.0;
float starfieldThreshold2 = 40.0;

// Updates shader parameters to show the planet atmosphere
// in the right place
void spaceUpdate(int spaceId, float dt)
{
  (void)dt;
  if (warp_enabled)
    warp_enabled = FALSE;
  // if (warp_enabled)
  // {
  //   warp_speed += warp_acceleration * dt;
  //   warp_position += warp_speed;
  //   if (warp_position >= 5.0)
  //   {
  //     warp_position = 0.0;
  //     warp_speed = 0.0;
  //     warp_enabled = FALSE;
  //   }
  // }

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);

  float viewX, viewY;
  float viewWidth, viewHeight;
  float viewAngle;
  microViewGetCenter(&viewX, &viewY);
  microViewGetSize(&viewWidth, &viewHeight);
  viewAngle = microViewGetRotation();

  float normPlanetX = ((planetX - viewX) * 2.0) / viewHeight;
  float normPlanetY = ((planetY - viewY) * 2.0) / viewHeight;

  microShaderApply(space_shader_id);

  // Update rotation center
  float rotationCenterX = 0.0;
  float playerW, playerH;
  PlayerGetSize(&playerW, &playerH);
  float rotationCenterY = ((PlanetGetRadius() - GROUND_HEIGHT + playerH / 2) *
                           2) /
                          viewHeight;
  microShaderSetUniform("view_center", rotationCenterX, rotationCenterY);

  // Update planet center
  microShaderSetUniform("planet_center", normPlanetX, normPlanetY);
  microShaderSetUniform("view_angle", viewAngle);
  microShaderSetUniform("position", warp_position);
  microShaderApply(0);

  CShadedCanvas *canvas = microECSEntityGetComponent(spaceId, cid_shadedCanvas);
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
  float canvas_space_height = SPACE_CANVAS_HEIGHT;
  float canvas_space_width = viewWidth * (canvas_space_height / viewHeight);
  float planet_radius = 2 * PlanetGetRadius() / viewHeight;
  float view_angle = 0.0;

  float planetX, planetY;
  PlanetGetPos(&planetX, &planetY);

  // Load shader and apply parameters
  space_shader_id = microShaderLoadFromFile("./res/shaders/dummy_vert.glsl",
                                            "./res/shaders/space.glsl");
  assert(space_shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(space_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_space_width, canvas_space_height);
  microShaderSetUniform("position", warp_position);

  float planetCenterDist = sqrt(pow(viewX - planetX, 2) +
                                pow(viewY - planetY, 2));
  float rotationCenterX = 0.0;
  float rotationCenterY = (planetCenterDist) / viewHeight;
  microShaderSetUniform("view_center", rotationCenterX, rotationCenterY);
  microShaderSetUniform("view_angle", view_angle);
  microShaderSetUniform("planet_radius", planet_radius);
  microShaderSetUniform("planet_center", 0.0, (viewY - planetY) / viewHeight);
  microShaderSetUniform("nebulaColor", nebulaColor[0], nebulaColor[1],
                        nebulaColor[2]);
  microShaderSetUniform("starfieldThreshold1", starfieldThreshold1);
  microShaderSetUniform("starfieldThreshold2", starfieldThreshold2);
  microShaderSetUniform("atmosphereMaxIntensity", atmosphereMaxIntensity);
  microShaderSetUniform("atmosphereDecay", atmosphereDecay);
  microShaderSetUniform("atmosphereColor", atmosphereColor[0],
                        atmosphereColor[1], atmosphereColor[2],
                        atmosphereColor[3]);

  microShaderApply(current_shader);

  // Position component
  CmpAddPosition(spaceId, 0, 0);

  // Shaded canvas component
  int canvasId = microCanvasCreate(canvas_space_width, canvas_space_height);
  CmpAddShaderCanvas(spaceId, canvas_space_width, canvas_space_height,
                     space_shader_id, canvasId);

  // Sprite component
  int textureId = microCanvasGetTextureId(canvasId);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  CmpAddSprite(spaceId, textureId, 0, 0, texWidth, texHeight);

  // Transform component
  float viewportWidth, viewportHeight;
  microViewGetViewport(&viewportWidth, &viewportHeight);
  CmpAddTransform(spaceId, viewportWidth, viewportHeight, 0, 0, 0);

  // Color component
  CmpAddColor(spaceId, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(spaceId, LAYER_SPACE, TRUE);
  CmpAddHud(spaceId);
  CmpAddUpdate(spaceId, spaceUpdate);
}

void SpaceSetAtmosphereMaxIntensity(float intensity)
{
  atmosphereMaxIntensity = intensity;
}

void SpaceSetAtmosphereDecay(float decay)
{
  atmosphereDecay = decay;
}

void SpaceSetAtmosphereColor(float r, float g, float b)
{
  atmosphereColor[0] = r;
  atmosphereColor[1] = g;
  atmosphereColor[2] = b;
}

void SpaceSetStarfieldTh1(float threshold)
{
  starfieldThreshold1 = threshold;
}

void SpaceSetStarfieldTh2(float threshold)
{
  starfieldThreshold2 = threshold; 
}

bool SpaceIsWarping()
{
  return warp_enabled;
}

void SpaceWarpDriveStart()
{
  warp_speed = 0.0;
  warp_acceleration = 0.0003;
  warp_position = 0.0;
  warp_enabled = TRUE;
}

void SpaceApplyParameters()
{
  int current_shader = microShaderGetCurrent();
  microShaderApply(space_shader_id);
  microShaderSetUniform("nebulaColor", nebulaColor[0], nebulaColor[1],
                        nebulaColor[2]);
  microShaderSetUniform("starfieldThreshold1", starfieldThreshold1);
  microShaderSetUniform("starfieldThreshold2", starfieldThreshold2);
  microShaderSetUniform("atmosphereMaxIntensity", atmosphereMaxIntensity);
  microShaderSetUniform("atmosphereDecay", atmosphereDecay);
  microShaderSetUniform("atmosphereColor", atmosphereColor[0],
                        atmosphereColor[1], atmosphereColor[2],
                        atmosphereColor[3]);
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);
  float planet_radius = 2 * PlanetGetRadius() / viewHeight;
  microShaderSetUniform("planet_radius", planet_radius);
  microShaderApply(current_shader);
}
