#include "Planet.h"
#include "../components/LogicComponents.h"
#include "../components/MotionComponents.h"
#include "../components/RenderingComponents.h"
#include "../micro/ECS.h"
#include "../micro/Graphics.h"
#include "../micro/Physics.h"
#include "../micro/Resources.h"
#include "../misc/collision.h"
#include "../util/perlin_noise.h"
#include "../util/debug.h"
#include "../util/vector.h"
#include "Cave.h"
#include "Space.h"
#include "Explosion.h"
#include "Resource.h"
#include <assert.h>

// Resources management
int planet_id = -1;
int planet_texture_id = -1;
int planet_shader_id = -1;
int planet_canvas_id = -1;
int planet_body_id = -1;
unsigned char* planet_texture = NULL;
int shadow_id = -1;
int shadow_shader_id = -1;
int shadow_canvas_id = -1;
Vector planet_entities;

// Planet fixed parameters
float planetX = 0.0;
float planetY = 0.0;
float planetScaleFactor = 2.0;
float planetLightDepth = 140;
float planetSurfaceDepth = GROUND_HEIGHT / 2.0;

// Planet genertion parameters 
PlanetType planetType = PLANET_RED_ROCKY;
float planetRadius = 400;


void PlanetCollision(int entityId, int otherEntityId)
{
  (void)entityId; // unused 
  CBody *body = CmpGetBody(otherEntityId);
  float vx, vy;
  microPhysicsBodyGetVelocity(body->body_id, &vx, &vy);
  f32 speed = sqrt(vx * vx + vy * vy);
  if (speed > 300.0)
  {
    CPosition *pos = CmpGetPosition(otherEntityId);
    makeExplostionProjectileGroundHit(pos->x, pos->y, vx, vy);
  }
}

float PlanetGetRadius()
{
  return planetRadius * planetScaleFactor / 2.0;
}

void PlanetGetPos(float *x, float *y)
{
  *x = planetX;
  *y = planetY;
}

void PlanetGetSurfacePosition(float angle, float offset, int *x, int *y)
{
  int sx = planetX +
           (PlanetGetRadius() - planetSurfaceDepth - offset) * cos(angle);
  int sy = planetY +
           (PlanetGetRadius() - planetSurfaceDepth - offset) * sin(angle);
  *x = sx;
  *y = sy;
}

// Updates shader parameters to show the shadows at the planet position
void planetUpdate(int planetId, float dt)
{
  (void)dt; // unused parameter

  CPosition *position = CmpGetPosition(planetId);

  float viewX, viewY;
  float viewAngle;
  microViewGetCenter(&viewX, &viewY);
  viewAngle = microViewGetRotation();

  float windowWidth, windowHeight;
  microViewGetSize(&windowWidth, &windowHeight);

  float normPlanetX = ((position->x - viewX) * 2.0) / windowHeight;
  float normPlanetY = ((position->y - viewY) * 2.0) / windowHeight;

  // float normViewX = viewX / windowHeight;
  // float normViewY = -viewY / windowHeight;

  microShaderApply(shadow_shader_id);
  microShaderSetUniform("planet_center", normPlanetX, normPlanetY);
  // microShaderSetUniform("view_center", normViewX, normViewY);
  microShaderSetUniform("view_angle", viewAngle);
  microShaderApply(0);

  CShadedCanvas *canvas = CmpGetShadedCanvas(shadow_id);
  canvas->needsUpdate = 1;
}

unsigned char *makePlanetTextureRedPlanet(int width, int height)
{
  unsigned char *texture = (unsigned char *)malloc(width * height * 4);
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      float distX = (float)x - (float)width / 2.0;
      float distY = (float)y - (float)height / 2.0;
      float distNorm = sqrt(distX * distX + distY * distY) /
                       ((float)width * 0.5);
      // distNorm += noise2((float)x / 100.0, (float)y / 100.0) * 0.04;

      // Compute color
      float f = 40.0;
      float r = (noise2((float)x / f, (float)y / f) + 1.0) * 0.5;
      float r2 = ((double)rand() / (double)RAND_MAX - 0.5) * 2.0;
      int r_quant = (0.3 + r * 0.2 + r2 * 0.15) * 255;
      int r_q_reduced = r_quant;
      if (r < 0.35)
      {
        texture[(y * width + x) * 4 + 0] = r_q_reduced;
        texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
        texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
      }
      else if (r < 0.6)
      {
        float n = (r - 0.35) / 0.25;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.9;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.9;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
      }
      else if (r < 0.85)
      {
        float n = (r - 0.6) / 0.2;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.9;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.9;
        }
      }
      else if (r < 0.95)
      {
        float n = (r - 0.85) / 0.1;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.3;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.3;
          texture[(y * width + x) * 4 + 2] = r_q_reduced;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
      }
      else
      {
        texture[(y * width + x) * 4 + 0] = r_q_reduced;
        texture[(y * width + x) * 4 + 1] = r_q_reduced;
        texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
      }

      // Compute alpha
      texture[(y * width + x) * 4 + 3] = 255 * (distNorm < 1.0);
    }
  }

  return texture;
}

unsigned char *makePlanetTextureAsteroid(int width, int height)
{
  unsigned char *texture = (unsigned char *)malloc(width * height * 4);
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      float distX = (float)x - (float)width / 2.0;
      float distY = (float)y - (float)height / 2.0;
      float distNorm = sqrt(distX * distX + distY * distY) /
                       ((float)width * 0.5);

      float angle = atan2(distX, distY);
      // distNorm += noise2((float)x / 100.0, (float)y / 100.0) * 0.04;

      // Compute color
      float f = 40.0;
      float r = (noise2((float)x / f, (float)y / f) + 1.0) * 0.5;
      r *= sin(distNorm * M_PI * 2.0 * 10.0) * 0.10 + 0.90;
      float r2 = ((double)rand() / (double)RAND_MAX - 0.5) * 2.0;
      int r_quant = (0.3 + r * 0.2 + r2 * 0.15) * 255;
      int r_q_reduced = r_quant;
      if (r < 0.35)
      {
        texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
        texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
        texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
      }
      else if (r < 0.6)
      {
        float n = (r - 0.35) / 0.25;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced;
          texture[(y * width + x) * 4 + 1] = r_q_reduced;
          texture[(y * width + x) * 4 + 2] = r_q_reduced;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
      }
      else if (r < 0.85)
      {
        float n = (r - 0.6) / 0.2;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced;
          texture[(y * width + x) * 4 + 1] = r_q_reduced;
          texture[(y * width + x) * 4 + 2] = r_q_reduced;
        }
      }
      else if (r < 0.95)
      {
        float n = (r - 0.85) / 0.1;
        float rf = (double)rand() / (double)RAND_MAX;
        if (rf < n)
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.3;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.3;
          texture[(y * width + x) * 4 + 2] = r_q_reduced;
        }
        else
        {
          texture[(y * width + x) * 4 + 0] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 1] = r_q_reduced * 0.5;
          texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
        }
      }
      else
      {
        texture[(y * width + x) * 4 + 0] = r_q_reduced;
        texture[(y * width + x) * 4 + 1] = r_q_reduced;
        texture[(y * width + x) * 4 + 2] = r_q_reduced * 0.5;
      }

      // Compute alpha
      texture[(y * width + x) * 4 + 3] = 255 * (distNorm < 1.0);
    }
  }

  return texture;
}

void PlanetFree(int planet_eid)
{
  CBody *body = CmpGetBody(planet_eid);
  microPhysicsBodyFree(body->body_id);
  if (planet_texture_id != -1)
    microTextureFree(planet_texture_id);
  if (planet_texture != NULL)
    free(planet_texture);
  CShadedCanvas *canvas = CmpGetShadedCanvas(planet_eid);
  microCanvasFree(canvas->canvasId);
  microShaderFree(canvas->shaderId);
  vector_free(&planet_entities);
}

void setupPlanetEntity()
{
  const u32 radius = 400;
  planetRadius = radius;

  planet_id = microECSEntityNew(NULL, PlanetFree);

  float canvas_planet_height = planetRadius * 2.0 / 2.0;
  float canvas_planet_width = planetRadius * 2.0 / 2.0;
  float planet_scale = planetScaleFactor;

  // Position component
  planetX = 0;
  planetY = 0;
  CmpAddPosition(planet_id, planetX, planetY);

  // Sprite component
  planet_texture = NULL;
  planet_texture_id = -1;
  int texWidth, texHeight;
  texWidth = radius * 2.0;
  texHeight = radius * 2.0;
  CmpAddSprite(planet_id, planet_texture_id, 0, 0, texWidth, texHeight);
  CmpAddTransform(planet_id, canvas_planet_width * planet_scale,
                  canvas_planet_height * planet_scale,
                  canvas_planet_width * planet_scale / 2.0,
                  canvas_planet_height * planet_scale / 2.0, 0.0);

  // Body component
  planet_body_id = microPhysicsBodyNewCircle(planet_id, 0, planetX, planetY,
                                             PlanetGetRadius() -
                                               GROUND_HEIGHT * 2.0,
                                             1000.0, 1, 0, 0.0, 1.0);
  microPhysicsBodySetCollisionCallback(planet_body_id, PlanetCollision);
  microPhysicsBodySetFilter(planet_body_id, COLLISION_GROUP_WORLD, COLLISION_MASK_ALL);
  CmpAddBody(planet_id, planet_body_id);

  CmpAddColor(planet_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(planet_id, 1, 1);
  CmpAddUpdate(planet_id, planetUpdate);
  
  // Allocate vector
  planet_entities = vector_create(sizeof(int));
}

void ShadowFree(int shadow_eid)
{
  CShadedCanvas *canvas = CmpGetShadedCanvas(shadow_eid);
  microCanvasFree(canvas->canvasId);
  microShaderFree(canvas->shaderId);
}

void setupShadow()
{
  shadow_id = microECSEntityNew(NULL, ShadowFree);

  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  float canvas_posteffect_height = 512.0;
  float canvas_posteffect_width = viewWidth *
                                  (canvas_posteffect_height / viewHeight);

  // Load shader and apply parameters
  shadow_shader_id = microShaderLoadFromFile("./res/shaders/dummy_vert.glsl",
                                             "./res/shaders/shadow.glsl");
  assert(shadow_shader_id != -1);
  int current_shader = microShaderGetCurrent();
  microShaderApply(shadow_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_posteffect_width,
                        canvas_posteffect_height);
  microShaderSetUniform("radius", (PlanetGetRadius() * 2.0) / viewHeight);
  microShaderSetUniform("lightDepth", planetLightDepth / viewHeight);
  microShaderSetUniform("planet_center", 0.0,
                        0.2 + PlanetGetRadius() / viewHeight);
  microShaderSetUniform("view_angle", 0.0);
  microShaderApply(current_shader);

  // Position component
  CmpAddPosition(shadow_id, planetX, planetY);

  // Shaded canvas component
  shadow_canvas_id = microCanvasCreate(canvas_posteffect_width,
                                       canvas_posteffect_height);
  CmpAddShaderCanvas(shadow_id, canvas_posteffect_width,
                     canvas_posteffect_height, shadow_shader_id,
                     shadow_canvas_id);

  // Sprite component
  int textureId = microCanvasGetTextureId(shadow_canvas_id);
  microTextureSetFilter(textureId, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(textureId, &texWidth, &texHeight);
  CmpAddSprite(shadow_id, textureId, 0, 0, texWidth, texHeight);
  float viewportWidth, viewportHeight;
  microViewGetViewport(&viewportWidth, &viewportHeight);
  CmpAddTransform(shadow_id, viewportWidth, viewportHeight, 0, 0, 0);
  CmpAddColor(shadow_id, 1.0, 1.0, 1.0, 1.0);
  CmpAddDrawable(shadow_id, 2, 1);
  CmpAddHud(shadow_id);
}

void PlanetGenerate()
{
  // Generate new planet parameters 
  planetType = rand() % PLANET_TYPES_COUNT;
  int rocksCount = 3;
  int cavesCount = 3;
  if (planetType == PLANET_ASTEROID)
    cavesCount = 0;
  
  if (planetType == PLANET_RED_ROCKY)
    planetRadius = 300 + rand() % 100;
  else if (planetType == PLANET_ASTEROID)
    planetRadius = 200 + rand() % 100;
  else
    assert(0);
  
  // Clear previous planet texture
  if (planet_texture_id != -1)
    microTextureFree(planet_texture_id);
  if (planet_texture != NULL)
    free(planet_texture);

  // Generate new planet texture
  float viewWidth, viewHeight;
  microViewGetSize(&viewWidth, &viewHeight);

  float canvas_planet_height = planetRadius * 2.0 / 2.0;
  float canvas_planet_width = planetRadius * 2.0 / 2.0;
  float planet_scale = planetScaleFactor;

  if (planetType == PLANET_RED_ROCKY)
    planet_texture = makePlanetTextureRedPlanet(canvas_planet_width,canvas_planet_height);
  else if (planetType == PLANET_ASTEROID)
    planet_texture = makePlanetTextureAsteroid(canvas_planet_width,canvas_planet_height);
  else
    assert(0);

  planet_texture_id = microTextureLoadFromMemory(planet_texture, canvas_planet_width,
                                             canvas_planet_height, 4,
                                             MICRO_FILTER_NEAREST);
  microTextureSetFilter(planet_texture_id, MICRO_FILTER_NEAREST);
  int texWidth, texHeight;
  microTextureGetSize(planet_texture_id, &texWidth, &texHeight);
  CSprite *sprite = CmpGetSprite(planet_id);
  sprite->textureId = planet_texture_id;
  sprite->tw = texWidth;
  sprite->th = texHeight;
  
  // Update sprite size
  CTransform *transform = CmpGetTransform(planet_id);
  transform->width = canvas_planet_width * planet_scale;
  transform->height = canvas_planet_height * planet_scale;
  transform->originX = canvas_planet_width * planet_scale / 2.0;
  transform->originY = canvas_planet_height * planet_scale / 2.0;
  transform->rotation = 0.0;
  
  // Update body size
  CBody *body = CmpGetBody(planet_id);
  microPhysicsBodyFree(body->body_id);
  planet_body_id = microPhysicsBodyNewCircle(planet_id, 0, planetX, planetY,
                                             PlanetGetRadius() -
                                               GROUND_HEIGHT * 2.0,
                                             1000.0, 1, 0, 0.0, 1.0);
  microPhysicsBodySetCollisionCallback(planet_body_id, PlanetCollision);
  microPhysicsBodySetFilter(planet_body_id, COLLISION_GROUP_WORLD, COLLISION_MASK_ALL);
  body->body_id = planet_body_id;
  
  // Update shadow
  float canvas_posteffect_height = 512.0;
  float canvas_posteffect_width = viewWidth *
                                  (canvas_posteffect_height / viewHeight);
  int current_shader = microShaderGetCurrent();
  microShaderApply(shadow_shader_id);
  microViewApply(); // send view matrix to shader
  microShaderSetUniform("resolution", canvas_posteffect_width,
                        canvas_posteffect_height);
  microShaderSetUniform("radius", (PlanetGetRadius() * 2.0) / viewHeight);
  microShaderSetUniform("lightDepth", planetLightDepth / viewHeight);
  microShaderSetUniform("planet_center", 0.0,
                        0.2 + PlanetGetRadius() / viewHeight);
  microShaderSetUniform("view_angle", 0.0);
  microShaderApply(current_shader);

  // Clear previously allocated entities
  u32 entitiesCount = planet_entities.size;
  while (entitiesCount--) {
    int eid = *(int*)vector_back(&planet_entities);
    if (microECSEntityIsAlive(eid))
      microECSEntityRemove(eid);
    vector_pop_back(&planet_entities);
  }
  
  // Add caves
  for (int i = 0; i < cavesCount; i++)
  {
    float angle = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
    int caveX, caveY;
    PlanetGetSurfacePosition(angle, 32.0 / 2.0 - 16, &caveX, &caveY);
    int eid = CaveAddEntity(caveX, caveY);
    vector_push_back(&planet_entities, &eid);
  }

  // Add rock
  for (int i = 0; i < rocksCount; i++)
  {
    float angle = (double)rand() / (double)RAND_MAX * 2.0 * M_PI;
    int rockX, rockY;
    PlanetGetSurfacePosition(angle, 16.0 / 2.0, &rockX, &rockY);
    int eid = ResourceAddEntity(rockX, rockY, RES_METAL);
    vector_push_back(&planet_entities, &eid);
  }

  // Update atmosphere and space location
  if (planetType == PLANET_RED_ROCKY)
  {
    SpaceSetAtmosphereMaxIntensity(0.5);
    SpaceSetAtmosphereDecay(0.90);
    SpaceSetAtmosphereColor(0.6, 0.6, 1.0);
  }
  else if (planetType == PLANET_ASTEROID)
  {
    SpaceSetAtmosphereMaxIntensity(0.1);
    SpaceSetAtmosphereDecay(0.99);
    SpaceSetAtmosphereColor(1.0, 1.0, 1.0);
  }
  else
    assert(0);
    
  //SpaceSetNebulaColor(0.57, 0.27, 0.41);
  SpaceSetStarfieldTh1(40.0);
  SpaceSetStarfieldTh2(40.0);

  SpaceApplyParameters();
}

void PlanetEntityAdd()
{
  setupPlanetEntity();
  setupShadow();
}

