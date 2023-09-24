#ifndef GRAPHICS_H
#define GRAPHICS_H

/////////////////////////////
// Texture
/////////////////////////////
enum MicroFilter
{
  MICRO_FILTER_NEAREST = 0,
  MICRO_FILTER_LINEAR = 1
};

extern int microBitmapLoadFromFile(const char *filepath, unsigned char **data,
                                   unsigned int *width, unsigned int *height,
                                   unsigned int *channels);
extern int microTextureLoadFromFile(const char *filepath);
extern int microTextureLoadFromMemory(const unsigned char *data,
                                      const unsigned int width,
                                      const unsigned int height,
                                      const unsigned int channels,
                                      const enum MicroFilter filter);
extern void microTextureGetSize(const int textureId, int *width, int *height);
extern void microTextureSetFilter(const int textureId,
                                  const enum MicroFilter filter);
extern void microTextureFree(const int textureId);

/////////////////////////////
/// Texture Atlas
/////////////////////////////
typedef struct
{
  int x, y, w, h;
} MicroTextureSource;
extern int microTextureAtlasLoadFromPath(const char *filepath);
extern MicroTextureSource microTextureAtlasGetRegion(int textureAtlasId,
                                                     const char *name);
extern int microTextureAtlasGetTextureId(int textureAtlasId);
extern void microTextureAtlasFree(int textureAtlasId);

/////////////////////////////
/// Animation
/////////////////////////////
extern int microAnimationCreateFromFrames(char *name, int *frames,
                                          int framesCount);
extern int microAnimationGet(char *name);
extern const char *microAnimationGetName(int animationId);
extern MicroTextureSource microAnimationGetFrame(int animationId, int frameId,
                                                 int flipX, int flipY);
extern int microAnimationGetFramesCount(int animationId);
extern void microAnimationFree(int animationId);
extern void microAnimationFreeAll();

////////////////////////////
/// Font
////////////////////////////
extern int microFontLoadFromFile(const char *filepath, unsigned int fontSize,
                                 int filter);
extern int microFontGetTextureId(int fontId);
extern int microFontGetSize(int fontId);
extern int microFontGetLineHeight(int fontId);
extern int microFontGetTextWidth(int fontId, const char *text);
extern void microFontFree(int fontId);
extern void microFontFreeAll();

////////////////////////////
/// Particle emitter
////////////////////////////
typedef struct
{
  unsigned char alive;
  float x, y;
  float life;
  float alpha;
  float scale;

  int textureId;
  float tx, ty, tw, th;
  float vx, vy;
  float maxLife;
  float rotation;
  float rotationSpeed;
  float startScale;
  float endScale;
  float startAlpha;
  float endAlpha;
} MicroParticle;
extern int microParticleEmitterCreateSteady(
  int x, int y, float emissionRate, MicroParticle (*generationFunc)(int));
extern int microParticleEmitterCreateExplosion(
  int x, int y, int particlesCount, MicroParticle (*generationFunc)(int));
extern void microParticleEmitterSetPosition(int emitterId, int x, int y);
extern void microParticleEmitterSetSize(int emitterId, int width, int height);
extern void microParticleEmitterSetEmissionRate(int emitterId,
                                                float emissionRate);
extern void microParticleEmitterSetGenerationFunc(
  int emitterId, MicroParticle (*generationFunc)(int));
extern void microParticleEmitterGetPosition(int emitterId, int *x, int *y);
extern float microParticleEmitterGetEmissionRate(int emitterId);
extern void microParticleEmitterDraw(int emitterId);
extern void microParticleEmittersUpdate(float dt);
extern void microParticleEmitterRemove(int emitterId);
extern void microParticleEmitterRemoveAll();
extern int microParticlesCount();

/////////////////////////////
// View
/////////////////////////////
typedef struct
{
  float viewportX, viewportY;
  float viewportWidth, viewportHeight;
  float centerX, centerY;
  float width, height;
  float rotation;
  int flipY;
} MicroView;
extern void microViewSet(MicroView view);
extern MicroView microViewGet();
extern void microViewApply();
extern void microViewFlipY(int flipY);
extern void microViewSetViewport(float x, float y, float width, float height);
extern void microViewSetCenter(float x, float y);
extern void microViewSetSize(float width, float height);
extern void microViewSetRotation(float rotation);

// Get view center
extern void microViewGetCenter(float *centerX, float *centerY);

// Get view size
extern void microViewGetSize(float *width, float *height);

// Get viewport
extern void microViewGetViewport(float *width, float *height);

// Get view rotation
extern float microViewGetRotation();

// Transform world coordinates to screen coordinates
extern void microViewPointWorldToScreen(float x, float y, float *outX,
                                        float *outY);

// Transform screen coordinates to world coordinates
extern void microViewPointScreenToWorld(float x, float y, float *outX,
                                        float *outY);

/////////////////////////////
// Shader
/////////////////////////////

// Load shader from file
extern int microShaderLoadFromFile(const char *vertexShaderPath,
                                   const char *fragmentShaderPath);

// Load shader from source code
extern int microShaderLoadFromSource(const char *vertexShaderSrc,
                                     const char *fragmentShaderSrc);

// Get shader program id
extern int microShaderGetProgramID(int shaderId);

// Free shader
extern void microShaderFree(int shaderId);

// Apply shader
extern void microShaderApply(int shaderId);

// Get current shader id
extern int microShaderGetCurrent();

// Set uniform values (variable number of arguments)
extern void microShaderSetUniform(const char *name, ...);

// Set uniform matrix 4x4
extern void microShaderSetMatrix4(const char *name, float *matrix);

// Get uniform value
extern void microShaderGetUniform1(const char *name, double *v1);

// Get uniform 2D vector
extern void microShaderGetUniform2(const char *name, double *v1, double *v2);

// Get uniform 3D vector
extern void microShaderGetUniform3(const char *name, double *v1, double *v2,
                                   double *v3);

// Get uniform 4D vector
extern void microShaderGetUniform4(const char *name, double *v1, double *v2,
                                   double *v3, double *v4);

/////////////////////////////
// Canvas
/////////////////////////////

// Create canvas with specified width and height
extern int microCanvasCreate(int width, int height);

// Get canvas texture id
extern int microCanvasGetTextureId(int canvasId);

// Free canvas
extern void microCanvasFree(int canvasId);

/////////////////////////////
// Graphics
/////////////////////////////
typedef struct RenderingDebugInfo
{
  int drawCalls;
  int triangles;
  int textureSwitches;
  int shaderSwitches;
} RenderingDebugInfo;

// Initializes graphics system
extern void microGraphicsInit();

// Frees all memory used by graphics system
extern void microGraphicsQuit();

// Clears screen
extern void microGraphicsClear();

// Draws geometry still in queue to screen
extern void microGraphicsDisplay();

// Set rendering target to screen
extern void microGraphicsRenderToScreen();

// Set rendering target to canvas
extern void microGraphicsRenderToCanvas(int canvasId);

// Draw rectangle
extern void microGraphicsDrawRect(int textureId, float tx, float ty, float tw,
                                  float th, float x, float y, float w, float h,
                                  float r, float g, float b, float a);

// Draw rectangle with rotation
extern void microGraphicsDrawRectRot(int textureId, float tx, float ty,
                                     float tw, float th, float x, float y,
                                     float w, float h, float originX,
                                     float originY, float rotation, float r,
                                     float g, float b, float a);

// Draw text with font
extern void microGraphicsDrawText(int fontId, const char *text, float x,
                                  float y, float lineSpacing, float r, float g,
                                  float b, float a);

// Get rendering debug info
extern RenderingDebugInfo microGetRenderingDebugInfo();

// Reset rendering debug info
extern void microRenderingDebugInfoClear();

// Swap buffers (finalizes frame)
extern void microSwapBuffers();

// CAP framerate and returns delta time in seconds
extern float microGraphicsDelayToNextFrame(float target_fps);

#endif
