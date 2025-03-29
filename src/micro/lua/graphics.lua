local ffi = require("ffi")
local lib = require("src/lua/libmicro") -- your dynamic library

ffi.cdef([[
/* Texture */
enum MicroFilter{
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

/* Texture Atlas */
typedef struct {
  int x, y, w, h;
} MicroTextureRegion;
int microAtlasLoadFromPath(const char *resName, const char *filepath);
MicroTextureRegion microAtlasGetRegion(int textureAtlasId, const char *name);
int microAtlasGetTextureId(int textureAtlasId);
int microAtlasGet(const char *resName);
void microAtlasFree(int textureAtlasId);

/* Animation */
int microAnimationCreateFromFrames(char *name, int *frames, int framesCount);
int microAnimationGet(char *name);
const char *microAnimationGetName(int animationId);
MicroTextureRegion microAnimationGetFrame(int animationId, int frameId,
                                          int flipX, int flipY);
int microAnimationGetFramesCount(int animationId);
void microAnimationFree(int animationId);
void microAnimationFreeAll();

/* Font */
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

/* Particle emitter */
typedef struct {
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

/* View */
typedef struct {
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
void microViewGetCenter(float *centerX, float *centerY);
void microViewGetSize(float *width, float *height);
void microViewGetViewport(float *width, float *height);
float microViewGetRotation();
void microViewPointWorldToScreen(float x, float y, float *outX, float *outY);
void microViewPointScreenToWorld(float x, float y, float *outX, float *outY);

/* Shader */
int microShaderLoadFromFile(const char *resName, const char *vertexShaderPath,
                            const char *fragmentShaderPath);
int microShaderLoadFromSource(const char *resName, const char *vertexShaderSrc,
                              const char *fragmentShaderSrc);
int microShaderGetProgramID(int shaderId);
int microShaderGet(const char *resName);
void microShaderFree(int shaderId);
void microShaderApply(int shaderId);
void microShaderApplyDefault();
int microShaderGetCurrent();
int microShaderGetAttributeLocation(int shaderId, const char *name);
int microShaderGetUniformLocation(int shaderId, const char *name);
void microShaderSetUniformf(const char *name, ...);
void microShaderSetUniformi(const char *name, ...);
void microShaderSetMatrix4(const char *name, float *matrix);
void microShaderGetUniform1(const char *name, double *v1);
void microShaderGetUniform2(const char *name, double *v1, double *v2);
void microShaderGetUniform3(const char *name, double *v1, double *v2,
                            double *v3);
void microShaderGetUniform4(const char *name, double *v1, double *v2,
                            double *v3, double *v4);

/* Canvas */
int microCanvasCreate(int width, int height);
int microCanvasGetTextureId(int canvasId);
void microCanvasFree(int canvasId);

/* Lighting */
int microLightAdd(float cx, float cy, float radius, float intensity);
void microLightSetPosition(int lightId, float x, float y);
void microLightSetRadius(int lightId, float radius);
void microLightSetIntensity(int lightId, float intensity);
void microLightSetActive(int lightId, int is_active);
void microLightRemove(int lightId);
void microLightRemoveAll();
void microLightsUpdateTexture();
int microLightsGetTextureId();
int microLightsGetCount();
int microLightsGetActiveCount();
void microLightsSetAmbientIntensity(float intensity);
float microLightsGetAmbientIntensity();

/* Graphics - VAO/VBO */
typedef enum {
  MICRO_BYTE,
  MICRO_UNSIGNED_BYTE,
  MICRO_SHORT,
  MICRO_UNSIGNED_SHORT,
  MICRO_INT,
  MICRO_UNSIGNED_INT,
  MICRO_FLOAT,
  MICRO_DOUBLE
} MicroAttributeType;

typedef enum {
  MICRO_STATIC_DRAW,
  MICRO_DYNAMIC_DRAW,
  MICRO_STREAM_DRAW
} MicroVAODrawType;

typedef struct {
  int vbo_id;
  bool consume_vbo;
  const char *name;
  int components;
  MicroAttributeType type;
  int stride;
  int divisor;
  void *offset_bytes;
  bool normalized;
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

/* Graphics - Text rendering and debug info */
typedef enum TextAlignment {
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_CENTER = 1,
  TEXT_ALIGN_RIGHT = 2,
} TextAlignment;
typedef struct RenderingDebugInfo {
  int drawCalls;
  int vertices;
  int textureSwitches;
  int shaderSwitches;
  int bytesSent;
} RenderingDebugInfo;

int microGraphicsInit();
void microGraphicsQuit();
void microGraphicsClear();
void microGraphicsClearColor(float r, float g, float b, float a);
void microGraphicsDisplay();
void microGraphicsRenderToScreen();
void microGraphicsRenderToCanvas(int canvasId);
void microGraphicsDrawSprite(int textureId, float tx, float ty, float tw,
                             float th, float x, float y, float w, float h,
                             float originX, float originY, float rotation,
                             unsigned char r, unsigned char g, unsigned char b,
                             unsigned char a);
void microGraphicsDrawText(int fontId, const char *text, float x, float y,
                           float lineSpacing, float scale, TextAlignment align,
                           int maxLineWidth, unsigned char r, unsigned char g,
                           unsigned char b, unsigned char a);
RenderingDebugInfo microGetRenderingDebugInfo();
void microRenderingDebugInfoClear();
void microSwapBuffers();
float microGraphicsDelayToNextFrame(float target_fps);
]])

local Gfx = {}

--[[-------------------------------------------------------------------------
    Enums
  -------------------------------------------------------------------------]]
--- MicroFilter enumeration.
-- @type MicroFilter
Gfx.MicroFilter = {
	NEAREST = 0,
	LINEAR = 1,
}

--- MicroVAODrawType enumeration.
-- @type MicroVAODrawType
Gfx.MicroVAODrawType = {
	STATIC_DRAW = 0,
	DYNAMIC_DRAW = 1,
	STREAM_DRAW = 2,
}

--- MicroAttributeType enumeration.
-- @type MicroAttributeType
Gfx.MicroAttributeType = {
	BYTE = 0,
	UNSIGNED_BYTE = 1,
	SHORT = 2,
	UNSIGNED_SHORT = 3,
	INT = 4,
	UNSIGNED_INT = 5,
	FLOAT = 6,
	DOUBLE = 7,
}

--- TextAlignment enumeration.
-- @type TextAlignment
Gfx.TextAlignment = {
	LEFT = 0,
	CENTER = 1,
	RIGHT = 2,
}

--[[-------------------------------------------------------------------------
    Texture Functions
  -------------------------------------------------------------------------]]
--- Loads a bitmap from a file.
-- @param filepath string path to the file.
-- @return ret number (0 on success), data pointer, width number, height number, channels number.
function Gfx.loadBitmapFromFile(filepath)
	local dataPtr = ffi.new("unsigned char*[1]")
	local width = ffi.new("unsigned int[1]")
	local height = ffi.new("unsigned int[1]")
	local channels = ffi.new("unsigned int[1]")
	local ret = lib.microBitmapLoadFromFile(filepath, dataPtr, width, height, channels)
	return ret, dataPtr[0], tonumber(width[0]), tonumber(height[0]), tonumber(channels[0])
end

--- Loads a texture from file.
-- @param resName string resource name.
-- @param filepath string path to the texture file.
-- @return textureId number.
function Gfx.loadTextureFromFile(resName, filepath)
	return lib.microTextureLoadFromFile(resName, filepath)
end

--- Loads a texture from memory.
-- @param resName string resource name.
-- @param data userdata pointer to pixel data.
-- @param width number image width.
-- @param height number image height.
-- @param channels number number of channels.
-- @param filter MicroFilter filtering mode.
-- @param textureUnitId number texture unit identifier.
-- @return textureId number.
function Gfx.loadTextureFromMemory(resName, data, width, height, channels, filter, textureUnitId)
	return lib.microTextureLoadFromMemory(resName, data, width, height, channels, filter, textureUnitId)
end

--- Submits new texture data.
-- @param textureId number.
-- @param startX number.
-- @param startY number.
-- @param width number.
-- @param height number.
-- @param data userdata pointer to new data.
function Gfx.textureSubmitData(textureId, startX, startY, width, height, data)
	lib.microTextureSubmitData(textureId, startX, startY, width, height, data)
end

--- Retrieves the size of a texture.
-- @param textureId number.
-- @return width number, height number.
function Gfx.getTextureSize(textureId)
	local width = ffi.new("int[1]")
	local height = ffi.new("int[1]")
	lib.microTextureGetSize(textureId, width, height)
	return tonumber(width[0]), tonumber(height[0])
end

--- Gets a texture by resource name.
-- @param resName string.
-- @return textureId number.
function Gfx.getTexture(resName)
	return lib.microTextureGet(resName)
end

--- Sets the texture filtering mode.
-- @param textureId number.
-- @param filter MicroFilter.
function Gfx.setTextureFilter(textureId, filter)
	lib.microTextureSetFilter(textureId, filter)
end

--- Applies a texture to a texture unit.
-- @param textureId number.
-- @param textureUnitId number.
function Gfx.applyTexture(textureId, textureUnitId)
	lib.microTextureApply(textureId, textureUnitId)
end

--- Frees a texture.
-- @param textureId number.
function Gfx.freeTexture(textureId)
	lib.microTextureFree(textureId)
end

--[[-------------------------------------------------------------------------
    Texture Atlas Functions
  -------------------------------------------------------------------------]]
--- Loads a texture atlas from a file.
-- @param resName string resource name.
-- @param filepath string path to the atlas file.
-- @return atlasId number.
function Gfx.loadAtlas(resName, filepath)
	return lib.microAtlasLoadFromPath(resName, filepath)
end

--- Gets a region from a texture atlas.
-- @param atlasId number.
-- @param name string name of the region.
-- @return region table with fields x, y, w, h.
function Gfx.getAtlasRegion(atlasId, name)
	local region = lib.microAtlasGetRegion(atlasId, name)
	return { x = region.x, y = region.y, w = region.w, h = region.h }
end

--- Gets the texture ID of a texture atlas.
-- @param atlasId number.
-- @return textureId number.
function Gfx.getAtlasTextureId(atlasId)
	return lib.microAtlasGetTextureId(atlasId)
end

--- Gets a texture atlas by resource name.
-- @param resName string.
-- @return atlasId number.
function Gfx.getAtlas(resName)
	return lib.microAtlasGet(resName)
end

--- Frees a texture atlas.
-- @param atlasId number.
function Gfx.freeAtlas(atlasId)
	lib.microAtlasFree(atlasId)
end

--[[-------------------------------------------------------------------------
    Animation Functions
  -------------------------------------------------------------------------]]
--- Creates an animation from frames.
-- @param name string animation name.
-- @param frames table array of frame indices.
-- @return animationId number.
function Gfx.createAnimationFromFrames(name, frames)
	local cframes = ffi.new("int[?]", #frames, frames)
	return lib.microAnimationCreateFromFrames(name, cframes, #frames)
end

--- Gets an animation by name.
-- @param name string.
-- @return animationId number.
function Gfx.getAnimation(name)
	return lib.microAnimationGet(name)
end

--- Retrieves the name of an animation.
-- @param animationId number.
-- @return name string.
function Gfx.getAnimationName(animationId)
	return ffi.string(lib.microAnimationGetName(animationId))
end

--- Retrieves a specific frame of an animation.
-- @param animationId number.
-- @param frameId number.
-- @param flipX number (0 or 1).
-- @param flipY number (0 or 1).
-- @return region table with fields x, y, w, h.
function Gfx.getAnimationFrame(animationId, frameId, flipX, flipY)
	local region = lib.microAnimationGetFrame(animationId, frameId, flipX, flipY)
	return { x = region.x, y = region.y, w = region.w, h = region.h }
end

--- Gets the number of frames in an animation.
-- @param animationId number.
-- @return count number.
function Gfx.getAnimationFramesCount(animationId)
	return lib.microAnimationGetFramesCount(animationId)
end

--- Frees an animation.
-- @param animationId number.
function Gfx.freeAnimation(animationId)
	lib.microAnimationFree(animationId)
end

--- Frees all animations.
function Gfx.freeAllAnimations()
	lib.microAnimationFreeAll()
end

--[[-------------------------------------------------------------------------
    Font Functions
  -------------------------------------------------------------------------]]
--- Loads a TrueType font.
-- @param resName string resource name.
-- @param filepath string path to the TTF file.
-- @param fontSize number.
-- @param filter number filtering mode.
-- @return fontId number.
function Gfx.loadFontTTF(resName, filepath, fontSize, filter)
	return lib.microFontTTFLoad(resName, filepath, fontSize, filter)
end

--- Creates a bitmap font.
-- @param resName string resource name.
-- @param textureId number.
-- @param charWidth number.
-- @param charHeight number.
-- @param charCodeStart number.
-- @param charCodeEnd number.
-- @return fontId number.
function Gfx.createFontBitmap(resName, textureId, charWidth, charHeight, charCodeStart, charCodeEnd)
	return lib.microFontBitmapMake(resName, textureId, charWidth, charHeight, charCodeStart, charCodeEnd)
end

--- Creates a bitmap font from a patch.
-- @param resName string resource name.
-- @param textureId number.
-- @param texSource table with fields x, y, w, h.
-- @param charWidth number.
-- @param charHeight number.
-- @param charCodeStart number.
-- @param charCodeEnd number.
-- @return fontId number.
function Gfx.createFontBitmapFromPatch(resName, textureId, texSource, charWidth, charHeight, charCodeStart, charCodeEnd)
	local ctexSource =
		ffi.new("MicroTextureRegion", { x = texSource.x, y = texSource.y, w = texSource.w, h = texSource.h })
	return lib.microFontBitmapMakeFromPatch(
		resName,
		textureId,
		ctexSource,
		charWidth,
		charHeight,
		charCodeStart,
		charCodeEnd
	)
end

--- Gets the texture ID used by a font.
-- @param fontId number.
-- @return textureId number.
function Gfx.getFontTextureId(fontId)
	return lib.microFontGetTextureId(fontId)
end

--- Gets the font size.
-- @param fontId number.
-- @return size number.
function Gfx.getFontSize(fontId)
	return lib.microFontGetSize(fontId)
end

--- Gets the line height for a font.
-- @param fontId number.
-- @param scale number.
-- @return lineHeight number.
function Gfx.getFontLineHeight(fontId, scale)
	return lib.microFontGetLineHeight(fontId, scale)
end

--- Gets the width of a text string using the font.
-- @param fontId number.
-- @param text string.
-- @param scale number.
-- @return width number.
function Gfx.getTextWidth(fontId, text, scale)
	return lib.microFontGetTextWidth(fontId, text, scale)
end

--- Gets the height of a text string using the font.
-- @param fontId number.
-- @param text string.
-- @param scale number.
-- @param lineSpacing number.
-- @return height number.
function Gfx.getTextHeight(fontId, text, scale, lineSpacing)
	return lib.microFontGetTextHeigth(fontId, text, scale, lineSpacing)
end

--- Gets a font by resource name.
-- @param resName string.
-- @return fontId number.
function Gfx.getFont(resName)
	return lib.microFontGet(resName)
end

--- Frees a font.
-- @param fontId number.
function Gfx.freeFont(fontId)
	lib.microFontFree(fontId)
end

--- Frees all fonts.
function Gfx.freeAllFonts()
	lib.microFontFreeAll()
end

--[[-------------------------------------------------------------------------
    Particle Emitter Functions
  -------------------------------------------------------------------------]]
--- Creates a steady particle emitter.
-- @param x number.
-- @param y number.
-- @param emissionRate number.
-- @param generationFunc function that takes an integer and returns a MicroParticle.
-- @return emitterId number.
function Gfx.createSteadyEmitter(x, y, emissionRate, generationFunc)
	return lib.microParticleEmitterCreateSteady(x, y, emissionRate, generationFunc)
end

--- Creates an explosion particle emitter.
-- @param x number.
-- @param y number.
-- @param particlesCount number.
-- @param generationFunc function that takes an integer and returns a MicroParticle.
-- @return emitterId number.
function Gfx.createExplosionEmitter(x, y, particlesCount, generationFunc)
	return lib.microParticleEmitterCreateExplosion(x, y, particlesCount, generationFunc)
end

--- Sets the position of a particle emitter.
-- @param emitterId number.
-- @param x number.
-- @param y number.
function Gfx.setEmitterPosition(emitterId, x, y)
	lib.microParticleEmitterSetPosition(emitterId, x, y)
end

--- Sets the size of a particle emitter.
-- @param emitterId number.
-- @param width number.
-- @param height number.
function Gfx.setEmitterSize(emitterId, width, height)
	lib.microParticleEmitterSetSize(emitterId, width, height)
end

--- Sets the emission rate of a particle emitter.
-- @param emitterId number.
-- @param emissionRate number.
function Gfx.setEmitterEmissionRate(emitterId, emissionRate)
	lib.microParticleEmitterSetEmissionRate(emitterId, emissionRate)
end

--- Updates all particle emitters.
-- @param dt number delta time.
function Gfx.updateEmitters(dt)
	lib.microParticleEmittersUpdate(dt)
end

--- Removes a particle emitter.
-- @param emitterId number.
function Gfx.removeEmitter(emitterId)
	lib.microParticleEmitterRemove(emitterId)
end

--- Removes all particle emitters.
function Gfx.removeAllEmitters()
	lib.microParticleEmitterRemoveAll()
end

--- Returns the total number of particles.
-- @return count number.
function Gfx.getParticlesCount()
	return lib.microParticlesCount()
end

--[[-------------------------------------------------------------------------
    View Functions
  -------------------------------------------------------------------------]]
--- Sets the current view.
--- @param viewportX number
--- @param viewportY number
--- @param viewportWidth number
--- @param viewportHeight number
--- @param centerX number|nil
--- @param centerY number|nil
--- @param width number|nil
--- @param height number|nil
--- @param rotation number|nil
--- @param flipY boolean|nil (0 or 1).
function Gfx.setView(
	viewportX,
	viewportY,
	viewportWidth,
	viewportHeight,
	centerX,
	centerY,
	width,
	height,
	rotation,
	flipY
)
	local cview = ffi.new("MicroView", {
		viewportX = viewportX,
		viewportY = viewportY,
		viewportWidth = viewportWidth,
		viewportHeight = viewportHeight,
		centerX = centerX or viewportWidth / 2,
		centerY = centerY or viewportHeight / 2,
		width = width or viewportWidth,
		height = height or viewportHeight,
		rotation = rotation or 0,
		flipY = flipY or false,
	})
	lib.microViewSet(cview)
end

--- Gets the current view.
-- @return view table.
function Gfx.getView()
	local cview = lib.microViewGet()
	return {
		viewportX = cview.viewportX,
		viewportY = cview.viewportY,
		viewportWidth = cview.viewportWidth,
		viewportHeight = cview.viewportHeight,
		centerX = cview.centerX,
		centerY = cview.centerY,
		width = cview.width,
		height = cview.height,
		rotation = cview.rotation,
		flipY = cview.flipY,
	}
end

--- Applies the current view.
function Gfx.applyView()
	lib.microViewApply()
end

--- Flips the view vertically.
-- @param flipY number (0 or 1).
function Gfx.flipViewY(flipY)
	lib.microViewFlipY(flipY)
end

--- Sets the view's viewport.
-- @param x number.
-- @param y number.
-- @param width number.
-- @param height number.
function Gfx.setViewViewport(x, y, width, height)
	lib.microViewSetViewport(x, y, width, height)
end

--- Sets the view's center.
-- @param x number.
-- @param y number.
function Gfx.setViewCenter(x, y)
	lib.microViewSetCenter(x, y)
end

--- Sets the view's size.
-- @param width number.
-- @param height number.
function Gfx.setViewSize(width, height)
	lib.microViewSetSize(width, height)
end

--- Sets the view's rotation.
-- @param rotation number.
function Gfx.setViewRotation(rotation)
	lib.microViewSetRotation(rotation)
end

--- Transforms a point from world coordinates to screen coordinates.
-- @param x number.
-- @param y number.
-- @return outX number, outY number.
function Gfx.worldToScreen(x, y)
	local outX = ffi.new("float[1]")
	local outY = ffi.new("float[1]")
	lib.microViewPointWorldToScreen(x, y, outX, outY)
	return outX[0], outY[0]
end

--- Transforms a point from screen coordinates to world coordinates.
-- @param x number.
-- @param y number.
-- @return outX number, outY number.
function Gfx.screenToWorld(x, y)
	local outX = ffi.new("float[1]")
	local outY = ffi.new("float[1]")
	lib.microViewPointScreenToWorld(x, y, outX, outY)
	return outX[0], outY[0]
end

--[[-------------------------------------------------------------------------
    Shader Functions
  -------------------------------------------------------------------------]]
--- Loads a shader from file.
-- @param resName string.
-- @param vertexShaderPath string.
-- @param fragmentShaderPath string.
-- @return shaderId number.
function Gfx.loadShaderFromFile(resName, vertexShaderPath, fragmentShaderPath)
	return lib.microShaderLoadFromFile(resName, vertexShaderPath, fragmentShaderPath)
end

--- Loads a shader from source.
-- @param resName string.
-- @param vertexShaderSrc string.
-- @param fragmentShaderSrc string.
-- @return shaderId number.
function Gfx.loadShaderFromSource(resName, vertexShaderSrc, fragmentShaderSrc)
	return lib.microShaderLoadFromSource(resName, vertexShaderSrc, fragmentShaderSrc)
end

--- Gets the program ID of a shader.
-- @param shaderId number.
-- @return programId number.
function Gfx.getShaderProgramID(shaderId)
	return lib.microShaderGetProgramID(shaderId)
end

--- Gets a shader by resource name.
-- @param resName string.
-- @return shaderId number.
function Gfx.getShader(resName)
	return lib.microShaderGet(resName)
end

--- Frees a shader.
-- @param shaderId number.
function Gfx.freeShader(shaderId)
	lib.microShaderFree(shaderId)
end

--- Applies a shader.
-- @param shaderId number.
function Gfx.applyShader(shaderId)
	lib.microShaderApply(shaderId)
end

--- Applies the default shader.
function Gfx.applyDefaultShader()
	lib.microShaderApplyDefault()
end

--- Gets the current shader ID.
-- @return shaderId number.
function Gfx.getCurrentShader()
	return lib.microShaderGetCurrent()
end

--[[-------------------------------------------------------------------------
    Canvas Functions
  -------------------------------------------------------------------------]]
--- Creates a canvas.
-- @param width number.
-- @param height number.
-- @return canvasId number.
function Gfx.createCanvas(width, height)
	return lib.microCanvasCreate(width, height)
end

--- Gets the texture ID of a canvas.
-- @param canvasId number.
-- @return textureId number.
function Gfx.getCanvasTextureId(canvasId)
	return lib.microCanvasGetTextureId(canvasId)
end

--- Frees a canvas.
-- @param canvasId number.
function Gfx.freeCanvas(canvasId)
	lib.microCanvasFree(canvasId)
end

--[[-------------------------------------------------------------------------
    Lighting Functions
  -------------------------------------------------------------------------]]
--- Adds a light to the scene.
-- @param cx number center x.
-- @param cy number center y.
-- @param radius number.
-- @param intensity number.
-- @return lightId number.
function Gfx.addLight(cx, cy, radius, intensity)
	return lib.microLightAdd(cx, cy, radius, intensity)
end

--- Sets the light's position.
-- @param lightId number.
-- @param x number.
-- @param y number.
function Gfx.setLightPosition(lightId, x, y)
	lib.microLightSetPosition(lightId, x, y)
end

--- Sets the light's radius.
-- @param lightId number.
-- @param radius number.
function Gfx.setLightRadius(lightId, radius)
	lib.microLightSetRadius(lightId, radius)
end

--- Sets the light's intensity.
-- @param lightId number.
-- @param intensity number.
function Gfx.setLightIntensity(lightId, intensity)
	lib.microLightSetIntensity(lightId, intensity)
end

--- Activates or deactivates a light.
-- @param lightId number.
-- @param isActive number (0 or 1).
function Gfx.setLightActive(lightId, isActive)
	lib.microLightSetActive(lightId, isActive)
end

--- Removes a light.
-- @param lightId number.
function Gfx.removeLight(lightId)
	lib.microLightRemove(lightId)
end

--- Removes all lights.
function Gfx.removeAllLights()
	lib.microLightRemoveAll()
end

--- Updates the lights texture.
function Gfx.updateLightsTexture()
	lib.microLightsUpdateTexture()
end

--- Gets the lights texture ID.
-- @return textureId number.
function Gfx.getLightsTextureId()
	return lib.microLightsGetTextureId()
end

--- Gets the total number of lights.
-- @return count number.
function Gfx.getLightsCount()
	return lib.microLightsGetCount()
end

--- Gets the count of active lights.
-- @return count number.
function Gfx.getActiveLightsCount()
	return lib.microLightsGetActiveCount()
end

--- Sets the ambient light intensity.
-- @param intensity number.
function Gfx.setAmbientIntensity(intensity)
	lib.microLightsSetAmbientIntensity(intensity)
end

--- Gets the ambient light intensity.
-- @return intensity number.
function Gfx.getAmbientIntensity()
	return lib.microLightsGetAmbientIntensity()
end

--[[-------------------------------------------------------------------------
    VAO and VBO Functions
  -------------------------------------------------------------------------]]
--- Creates a new VAO.
-- @param shaderId number shader id.
-- @param textureId number texture id.
-- @param vertexCount number vertex count.
-- @param instancesCount number instance count.
-- @param attributes table array of attribute tables.
-- Each attribute table should include:
--   vbo_id (number), consume_vbo (boolean), name (string), components (number),
--   type (number, see MicroAttributeType), stride (number), divisor (number),
--   offset (number) and normalized (boolean).
-- @return vaoId number.
function Gfx.newVAO(shaderId, textureId, vertexCount, instancesCount, attributes)
	local count = #attributes
	local attrArray = ffi.new("MicroAttributeData[?]", count)
	for i, attr in ipairs(attributes) do
		local idx = i - 1
		attrArray[idx].vbo_id = attr.vbo_id or 0
		attrArray[idx].consume_vbo = attr.consume_vbo or false
		attrArray[idx].name = attr.name or ""
		attrArray[idx].components = attr.components or 0
		attrArray[idx].type = attr.type or 0
		attrArray[idx].stride = attr.stride or 0
		attrArray[idx].divisor = attr.divisor or 0
		attrArray[idx].offset_bytes = ffi.cast("void*", attr.offset or 0)
		attrArray[idx].normalized = attr.normalized or false
	end
	return lib.microVAONew(shaderId, textureId, vertexCount, instancesCount, attrArray, count)
end

--- Submits data to a VAO attribute.
-- @param vaoId number.
-- @param attributeName string attribute name.
-- @param data userdata pointer to the data.
-- @param start number start index.
-- @param count number element count.
function Gfx.vaoSubmit(vaoId, attributeName, data, start, count)
	lib.microVAOSubmit(vaoId, attributeName, data, start, count)
end

--- Sets the draw range for a VAO.
-- @param vaoId number.
-- @param start number start index.
-- @param count number count.
-- @param instancesCount number number of instances.
-- @param baseInstance number base instance offset.
function Gfx.vaoSetDrawRange(vaoId, start, count, instancesCount, baseInstance)
	lib.microVAOSetDrawRange(vaoId, start, count, instancesCount, baseInstance)
end

--- Draws the VAO.
-- @param vaoId number.
function Gfx.drawVAO(vaoId)
	lib.microVAODraw(vaoId)
end

--- Frees the VAO.
-- @param vaoId number.
function Gfx.freeVAO(vaoId)
	lib.microVAOFree(vaoId)
end

--- Creates a new VBO.
-- @param size number size in bytes.
-- @param drawType number (use MicroVAODrawType, e.g. Gfx.MicroVAODrawType.STATIC_DRAW).
-- @param data userdata pointer to initial data (can be nil).
-- @return vboId number.
function Gfx.newVBO(size, drawType, data)
	return lib.microVBONew(size, drawType, data)
end

--- Submits data to a VBO.
-- @param vboId number.
-- @param data userdata pointer to the data.
-- @param start number start index.
-- @param count number element count.
function Gfx.vboSubmit(vboId, data, start, count)
	lib.microVBOSubmit(vboId, data, start, count)
end

--- Frees the VBO.
-- @param vboId number.
function Gfx.freeVBO(vboId)
	lib.microVBOFree(vboId)
end

--[[-------------------------------------------------------------------------
    Graphics (Rendering) Functions
  -------------------------------------------------------------------------]]
--- Initializes the graphics system.
-- @return status number (0 on success).
function Gfx.initGraphics()
	return lib.microGraphicsInit()
end

--- Quits the graphics system.
function Gfx.quitGraphics()
	lib.microGraphicsQuit()
end

--- Clears the screen.
function Gfx.clearGraphics()
	lib.microGraphicsClear()
end

--- Sets the clear color.
-- @param r number.
-- @param g number.
-- @param b number.
-- @param a number.
function Gfx.setClearColor(r, g, b, a)
	lib.microGraphicsClearColor(r, g, b, a)
end

--- Displays the rendered geometry.
function Gfx.displayGraphics()
	lib.microGraphicsDisplay()
end

--- Sets the rendering target to screen.
function Gfx.renderToScreen()
	lib.microGraphicsRenderToScreen()
end

--- Sets the rendering target to a canvas.
-- @param canvasId number.
function Gfx.renderToCanvas(canvasId)
	lib.microGraphicsRenderToCanvas(canvasId)
end

--- Draws a sprite.
-- @param textureId number.
-- @param tx number texture x.
-- @param ty number texture y.
-- @param tw number texture width.
-- @param th number texture height.
-- @param x number screen x.
-- @param y number screen y.
-- @param w number screen width.
-- @param h number screen height.
-- @param originX number.
-- @param originY number.
-- @param rotation number.
-- @param r number red.
-- @param g number green.
-- @param b number blue.
-- @param a number alpha.
function Gfx.drawSprite(textureId, tx, ty, tw, th, x, y, w, h, originX, originY, rotation, r, g, b, a)
	lib.microGraphicsDrawSprite(textureId, tx, ty, tw, th, x, y, w, h, originX, originY, rotation, r, g, b, a)
end

--- Draws text using a font.
-- @param fontId number.
-- @param text string.
-- @param x number.
-- @param y number.
-- @param lineSpacing number.
-- @param scale number.
-- @param align TextAlignment.
-- @param maxLineWidth number.
-- @param r number red.
-- @param g number green.
-- @param b number blue.
-- @param a number alpha.
function Gfx.drawText(fontId, text, x, y, lineSpacing, scale, align, maxLineWidth, r, g, b, a)
	lib.microGraphicsDrawText(fontId, text, x, y, lineSpacing, scale, align, maxLineWidth, r, g, b, a)
end

--- Gets rendering debug information.
-- @return info table with keys: drawCalls, vertices, textureSwitches, shaderSwitches, bytesSent.
function Gfx.getRenderingDebugInfo()
	local info = lib.microGetRenderingDebugInfo()
	return {
		drawCalls = info.drawCalls,
		vertices = info.vertices,
		textureSwitches = info.textureSwitches,
		shaderSwitches = info.shaderSwitches,
		bytesSent = info.bytesSent,
	}
end

--- Clears the rendering debug information.
function Gfx.clearRenderingDebugInfo()
	lib.microRenderingDebugInfoClear()
end

--- Swaps the buffers.
function Gfx.swapBuffers()
	lib.microSwapBuffers()
end

--- Caps the framerate and returns the delta time.
-- @param targetFPS number.
-- @return dt number.
function Gfx.delayToNextFrame(targetFPS)
	return lib.microGraphicsDelayToNextFrame(targetFPS)
end

return Gfx
