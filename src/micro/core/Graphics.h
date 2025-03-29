#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

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
int microTextureLoadFromFile(const char *resName, const char *filepath);
int microTextureLoadFromMemory(const char *resName, const unsigned char *data,
                               const unsigned int width,
                               const unsigned int height,
                               const unsigned int channels,
                               const enum MicroFilter filter,
                               const unsigned int textureUnitId);
int microTextureSubmitData(const int textureId, const unsigned int startX,
                           const unsigned int startY, const unsigned int width,
                           const unsigned int height,
                           const unsigned char *data);
void microTextureGetSize(const int textureId, int *width, int *height);
int microTextureGet(const char *resName);
void microTextureSetFilter(const int textureId, const enum MicroFilter filter);
void microTextureApply(const int textureId, const int textureUnitId);
void microTextureFree(const int textureId);

/////////////////////////////
/// Texture Atlas
/////////////////////////////
typedef struct
{
  int x, y, w, h;
} MicroTextureRegion;
int microAtlasLoadFromPath(const char *resName, const char *filepath);
MicroTextureRegion microAtlasGetRegion(int textureAtlasId, const char *name);
int microAtlasGetTextureId(int textureAtlasId);
int microAtlasGet(const char *resName);
void microAtlasFree(int textureAtlasId);

/////////////////////////////
/// Animation
/////////////////////////////
int microAnimationCreateFromFrames(char *name, int *frames, int framesCount);
int microAnimationGet(char *name);
const char *microAnimationGetName(int animationId);
MicroTextureRegion microAnimationGetFrame(int animationId, int frameId,
                                          int flipX, int flipY);
int microAnimationGetFramesCount(int animationId);
void microAnimationFree(int animationId);
void microAnimationFreeAll();

////////////////////////////
/// Font
////////////////////////////
int microFontTTFLoad(const char *resName, const char *filepath,
                     unsigned int fontSize, int filter);
int microFontBitmapMake(const char *resName, int textureId, int charWidth,
                        int charHeight, int charCodeStart, int charCodeEnd);
int microFontBitmapMakeFromPatch(const char *resName, int textureId,
                                 MicroTextureRegion texSource, int charWidth,
                                 int charHeight, int charCodeStart,
                                 int charCodeEnd);
int microFontGetTextureId(int fontId);
int microFontGetSize(int fontId);
int microFontGetLineHeight(int fontId, float scale);
int microFontGetTextWidth(int fontId, const char *text, float scale);
int microFontGetTextHeigth(int fontId, const char *text, float scale,
                           int lineSpacing);
int microFontGet(const char *resName);
void microFontFree(int fontId);
void microFontFreeAll();

////////////////////////////
/// Bitmap Font
////////////////////////////
// int microBitmapFontMake(int textureId, int charWidth, int charHeight,)

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
int microParticleEmitterCreateSteady(int x, int y, float emissionRate,
                                     MicroParticle (*generationFunc)(int));
int microParticleEmitterCreateExplosion(int x, int y, int particlesCount,
                                        MicroParticle (*generationFunc)(int));
void microParticleEmitterSetPosition(int emitterId, int x, int y);
void microParticleEmitterSetSize(int emitterId, int width, int height);
void microParticleEmitterSetEmissionRate(int emitterId, float emissionRate);
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
void microViewPointWorldToScreen(float x, float y, float *outX, float *outY);

// Transform screen coordinates to world coordinates
void microViewPointScreenToWorld(float x, float y, float *outX, float *outY);

/////////////////////////////
// Shader
/////////////////////////////

// Load shader from file
int microShaderLoadFromFile(const char *resName, const char *vertexShaderPath,
                            const char *fragmentShaderPath);

// Load shader from source code
int microShaderLoadFromSource(const char *resName, const char *vertexShaderSrc,
                              const char *fragmentShaderSrc);

// Get shader program id
int microShaderGetProgramID(int shaderId);

// Get shader id by name
int microShaderGet(const char *resName);

// Free shader
void microShaderFree(int shaderId);

// Apply shader
void microShaderApply(int shaderId);

// Apply default shader
void microShaderApplyDefault();

// Get current shader id
int microShaderGetCurrent();

// Get attribute location in shader
int microShaderGetAttributeLocation(int shaderId, const char *name);

// Get uniform location in shader
int microShaderGetUniformLocation(int shaderId, const char *name);

// Set uniform values (variable number of arguments)
void microShaderSetUniformf(const char *name, ...);

// Set uniform values (variable number of arguments)
void microShaderSetUniformi(const char *name, ...);

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
// Lighting
/////////////////////////////

// Add light to scene
int microLightAdd(float cx, float cy, float radius, float intensity);

// Set light position
void microLightSetPosition(int lightId, float x, float y);

// Set light scale
void microLightSetRadius(int lightId, float radius);

// Set light intensity
void microLightSetIntensity(int lightId, float intensity);

// Set light is active
void microLightSetActive(int lightId, int is_active);

// Remove light from scene
void microLightRemove(int lightId);

// Remove all lights from scene
void microLightRemoveAll();

// Recalculate lights texture
void microLightsUpdateTexture();

// Get lights texture id
int microLightsGetTextureId();

// Get lights count
int microLightsGetCount();

// Get active lights count
int microLightsGetActiveCount();

// Set ambient light intensity
void microLightsSetAmbientIntensity(float intensity);

// Get ambient light intensity
float microLightsGetAmbientIntensity();

/////////////////////////////
// Graphics
/////////////////////////////
typedef enum
{
  MICRO_BYTE,
  MICRO_UNSIGNED_BYTE,
  MICRO_SHORT,
  MICRO_UNSIGNED_SHORT,
  MICRO_INT,
  MICRO_UNSIGNED_INT,
  MICRO_FLOAT,
  MICRO_DOUBLE
} MicroAttributeType;

typedef enum
{
  MICRO_STATIC_DRAW,
  MICRO_DYNAMIC_DRAW,
  MICRO_STREAM_DRAW
} MicroVAODrawType;

typedef struct
{
  int vbo_id;              // VBO id
  bool consume_vbo;        // If true, VBO will be freed when VAO is freed
  const char *name;        // Attribute name in the shader
  int components;          // Number of components per vertex attribute
  MicroAttributeType type; // Attribute data type
  int stride;              // Byte offset between consecutive attributes
  int divisor; // Attribute divisor (0 for per-vertex, 1 for per-instance, ...)
  void *offset_bytes; // Byte offset of the attribute data
  bool normalized;    // If true, values will be normalized
} MicroAttributeData;

int microVAONew(int shaderId, int textureId, int vertexCount,
                int instancesCount, const MicroAttributeData *attributes,
                int attributesCount);
void microVAOSubmit(int vaoId, const char *attribute_name, const void *data,
                    int start, int count);
void microVAOSetDrawRange(int vaoId, int start, int count, int instancesCount,
                          int baseInstance);
void microVAODraw(int vaoId);
void microVAOFree(int vaoId);

unsigned int microVBONew(int size, MicroVAODrawType drawType, const void *data);
void microVBOSubmit(unsigned int vboId, const void *data, int start, int count);
void microVBOFree(unsigned int vboId);

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
  int vertices;
  int textureSwitches;
  int shaderSwitches;
  int bytesSent;
} RenderingDebugInfo;

// Initializes graphics system
int microGraphicsInit();

// Frees all memory used by graphics system
void microGraphicsQuit();

// Clears screen
void microGraphicsClear();

// Set clear color
void microGraphicsClearColor(float r, float g, float b, float a);

// Draws geometry still in queue to screen
void microGraphicsDisplay();

// Set rendering target to screen
void microGraphicsRenderToScreen();

// Set rendering target to canvas
void microGraphicsRenderToCanvas(int canvasId);

// Draw sprite
void microGraphicsDrawSprite(int textureId, float tx, float ty, float tw,
                             float th, float x, float y, float w, float h,
                             float originX, float originY, float rotation,
                             unsigned char r, unsigned char g, unsigned char b,
                             unsigned char a);

// Draw text with font
void microGraphicsDrawText(int fontId, const char *text, float x, float y,
                           float lineSpacing, float scale, TextAlignment align,
                           int maxLineWidth, unsigned char r, unsigned char g,
                           unsigned char b, unsigned char a);

// Get rendering debug info
RenderingDebugInfo microGetRenderingDebugInfo();

// Reset rendering debug info
void microRenderingDebugInfoClear();

// Swap buffers (finalizes frame)
void microSwapBuffers();

// CAP framerate and returns delta time in seconds
float microGraphicsDelayToNextFrame(float target_fps);

#endif
