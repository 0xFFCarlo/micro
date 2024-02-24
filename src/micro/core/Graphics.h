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

int microBitmapLoadFromFile(const char *filepath, unsigned char **data,
                                   unsigned int *width, unsigned int *height,
                                   unsigned int *channels);
int microTextureLoadFromFile(const char *filepath);
int microTextureLoadFromMemory(const unsigned char *data,
                                      const unsigned int width,
                                      const unsigned int height,
                                      const unsigned int channels,
                                      const enum MicroFilter filter);
void microTextureGetSize(const int textureId, int *width, int *height);
void microTextureSetFilter(const int textureId,
                                  const enum MicroFilter filter);
void microTextureFree(const int textureId);

/////////////////////////////
/// Texture Atlas
/////////////////////////////
typedef struct
{
  int x, y, w, h;
} MicroTextureSource;
int microTextureAtlasLoadFromPath(const char *filepath);
MicroTextureSource microTextureAtlasGetRegion(int textureAtlasId,
                                                     const char *name);
int microTextureAtlasGetTextureId(int textureAtlasId);
void microTextureAtlasFree(int textureAtlasId);

/////////////////////////////
/// Animation
/////////////////////////////
int microAnimationCreateFromFrames(char *name, int *frames,
                                          int framesCount);
int microAnimationGet(char *name);
const char *microAnimationGetName(int animationId);
MicroTextureSource microAnimationGetFrame(int animationId, int frameId,
                                                 int flipX, int flipY);
int microAnimationGetFramesCount(int animationId);
void microAnimationFree(int animationId);
void microAnimationFreeAll();

////////////////////////////
/// Font
////////////////////////////
int microFontLoadFromFile(const char *filepath, unsigned int fontSize,
                                 int filter);
int microFontGetTextureId(int fontId);
int microFontGetSize(int fontId);
int microFontGetLineHeight(int fontId);
int microFontGetTextWidth(int fontId, const char *text);
void microFontFree(int fontId);
void microFontFreeAll();

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
int microParticleEmitterCreateSteady(
  int x, int y, float emissionRate, MicroParticle (*generationFunc)(int));
int microParticleEmitterCreateExplosion(
  int x, int y, int particlesCount, MicroParticle (*generationFunc)(int));
void microParticleEmitterSetPosition(int emitterId, int x, int y);
void microParticleEmitterSetSize(int emitterId, int width, int height);
void microParticleEmitterSetEmissionRate(int emitterId,
                                                float emissionRate);
void microParticleEmitterSetGenerationFunc(
  int emitterId, MicroParticle (*generationFunc)(int));
void microParticleEmitterGetPosition(int emitterId, int *x, int *y);
float microParticleEmitterGetEmissionRate(int emitterId);
void microParticleEmitterDraw(int emitterId);
void microParticleEmittersUpdate(float dt);
void microParticleEmitterRemove(int emitterId);
void microParticleEmitterRemoveAll();
int microParticlesCount();

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
void microViewSet(MicroView view);
MicroView microViewGet();
void microViewApply();
void microViewFlipY(int flipY);
void microViewSetViewport(float x, float y, float width, float height);
void microViewSetCenter(float x, float y);
void microViewSetSize(float width, float height);
void microViewSetRotation(float rotation);

// Get view center
void microViewGetCenter(float *centerX, float *centerY);

// Get view size
void microViewGetSize(float *width, float *height);

// Get viewport
void microViewGetViewport(float *width, float *height);

// Get view rotation
float microViewGetRotation();

// Transform world coordinates to screen coordinates
void microViewPointWorldToScreen(float x, float y, float *outX,
                                        float *outY);

// Transform screen coordinates to world coordinates
void microViewPointScreenToWorld(float x, float y, float *outX,
                                        float *outY);

/////////////////////////////
// Shader
/////////////////////////////

// Load shader from file
int microShaderLoadFromFile(const char *vertexShaderPath,
                                   const char *fragmentShaderPath);

// Load shader from source code
int microShaderLoadFromSource(const char *vertexShaderSrc,
                                     const char *fragmentShaderSrc);

// Get shader program id
int microShaderGetProgramID(int shaderId);

// Free shader
void microShaderFree(int shaderId);

// Apply shader
void microShaderApply(int shaderId);

// Get current shader id
int microShaderGetCurrent();

// Set uniform values (variable number of arguments)
void microShaderSetUniform(const char *name, ...);

// Set uniform matrix 4x4
void microShaderSetMatrix4(const char *name, float *matrix);

// Get uniform value
void microShaderGetUniform1(const char *name, double *v1);

// Get uniform 2D vector
void microShaderGetUniform2(const char *name, double *v1, double *v2);

// Get uniform 3D vector
void microShaderGetUniform3(const char *name, double *v1, double *v2,
                                   double *v3);

// Get uniform 4D vector
void microShaderGetUniform4(const char *name, double *v1, double *v2,
                                   double *v3, double *v4);

/////////////////////////////
// Canvas
/////////////////////////////

// Create canvas with specified width and height
int microCanvasCreate(int width, int height);

// Get canvas texture id
int microCanvasGetTextureId(int canvasId);

// Free canvas
void microCanvasFree(int canvasId);

/////////////////////////////
// Graphics
/////////////////////////////
typedef enum TextAlignment
{
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_CENTER = 1,
  TEXT_ALIGN_RIGHT = 2,
} TextAlignment;
typedef struct RenderingDebugInfo
{
  int drawCalls;
  int triangles;
  int textureSwitches;
  int shaderSwitches;
} RenderingDebugInfo;

// Initializes graphics system
void microGraphicsInit();

// Frees all memory used by graphics system
void microGraphicsQuit();

// Clears screen
void microGraphicsClear();

// Draws geometry still in queue to screen
void microGraphicsDisplay();

// Set rendering target to screen
void microGraphicsRenderToScreen();

// Set rendering target to canvas
void microGraphicsRenderToCanvas(int canvasId);

// Draw rectangle
void microGraphicsDrawRect(int textureId, float tx, float ty, float tw,
                                  float th, float x, float y, float w, float h,
                                  float r, float g, float b, float a);

// Draw rectangle with rotation
void microGraphicsDrawRectRot(int textureId, float tx, float ty,
                                     float tw, float th, float x, float y,
                                     float w, float h, float originX,
                                     float originY, float rotation, float r,
                                     float g, float b, float a);

// Draw text with font
void microGraphicsDrawText(int fontId, const char *text, float x,
                                  float y, float lineSpacing,
                                  TextAlignment align, float r, float g,
                                  float b, float a);

// Get rendering debug info
RenderingDebugInfo microGetRenderingDebugInfo();

// Reset rendering debug info
void microRenderingDebugInfoClear();

// Swap buffers (finalizes frame)
void microSwapBuffers();

// CAP framerate and returns delta time in seconds
float microGraphicsDelayToNextFrame(float target_fps);

#endif
