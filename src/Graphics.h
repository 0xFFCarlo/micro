#ifndef GRAPHICS_H
#define GRAPHICS_H

#define MICRO_FILTER_NEAREST 0
#define MICRO_FILTER_LINEAR 1

/////////////////////////////
// Texture
/////////////////////////////
extern int microTextureLoadFromFile(const char *filepath);
extern int microTextureLoadFromMemory(const unsigned char *data,
    const unsigned int width, const unsigned int height,
    const unsigned int channels, const unsigned int filter);
extern void microTextureGetSize(int textureId, int *width, int *height);
extern void microTextureSetFilter(int textureId, int filter);
extern void microTextureFree(int textureId);


/////////////////////////////
/// Animation
/////////////////////////////
extern void microAnimationLoadFromFile(const char *csv_filepath);
extern int microAnimationCreate(char* name, int startX, int startY, int frameWidth, int frameHeight, int framesCount, float animationSpeed, int flipX, int flipY);
extern const char* microAnimationGetName(int animationId);
extern void microAnimationGetStart(int animationId, int *startX, int *startY);
extern void microAnimationGetFrameSize(int animationId, int *frameWidth, int *frameHeight);
extern int microAnimationGetFramesCount(int animationId);
extern float microAnimationGetSpeed(int animationId);
extern int microAnimationGetFlipX(int animationId);
extern int microAnimationGetFlipY(int animationId);
extern void microAnimationFree(int animationId);

////////////////////////////
/// Font
////////////////////////////
extern int microFontLoadFromFile(const char *filepath, unsigned int fontSize, int filter);
extern int microFontGetTextureId(int fontId);
extern int microFontGetSize(int fontId);
extern void microFontFree(int fontId);


/////////////////////////////
// View
/////////////////////////////
typedef struct {
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
extern void microViewGetCenter(float *centerX, float *centerY);
extern void microViewGetSize(float *width, float *height);
extern float microViewGetRotation();


/////////////////////////////
// Shader
/////////////////////////////
extern int microShaderLoadFromFile(const char *vertexShaderPath, const char *fragmentShaderPath);
extern int microShaderLoadFromSource(const char *vertexShaderSrc, const char *fragmentShaderSrc);
extern int microShaderGetProgramID(int shaderId);
extern void microShaderFree(int shaderId);
extern void microShaderApply(int shaderId);
extern int microShaderGetCurrent();
extern void microShaderSetUniform(const char *name, ...);
extern void microShaderSetMatrix4(const char *name, float *matrix);
extern void microShaderGetUniform1(const char *name, double* v1);
extern void microShaderGetUniform2(const char *name, double* v1, double* v2);
extern void microShaderGetUniform3(const char *name, double* v1, double* v2, double* v3);
extern void microShaderGetUniform4(const char *name, double* v1, double* v2, double* v3, double* v4);


/////////////////////////////
// Canvas
/////////////////////////////
extern int microCanvasCreate(int width, int height);
extern int microCanvasGetTextureId(int canvasId);
extern void microCanvasFree(int canvasId);


/////////////////////////////
// Graphics
/////////////////////////////
extern void microGraphicsInit();
extern void microGraphicsQuit();
extern void microGraphicsClear();
extern void microGraphicsDisplay();
extern void microGraphicsRenderToScreen();
extern void microGraphicsRenderToCanvas(int canvasId);
extern void microGraphicsDrawRect(
    int textureId, float tx, float ty,
    float tw, float th, float x, float y,
    float w, float h, float r, float g, float b, float a);
extern void microGraphicsDrawRectRot(
    int textureId, float tx, float ty, 
    float tw, float th,
    float x, float y, float w, float h,
    float originX, float originY,
    float rotation, float r, float g, float b, float a);
extern void microGraphicsDrawText(
    int fontId, const char *text,
    float x, float y, float lineSpacing, 
    float r, float g, float b, float a);

extern void microWindowGetSize(int *width, int *height);
extern void microSwapBuffers();


#endif
