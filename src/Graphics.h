#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL_video.h>

#define MICRO_FILTER_NEAREST 0
#define MICRO_FILTER_LINEAR 1

//texture
extern int microTextureLoadFromFile(const char *filepath);
extern int microTextureLoadFromMemory(const unsigned char *data, const unsigned int width, const unsigned int height, const unsigned int channels, const unsigned int filter);
extern void microTextureGetSize(int textureId, int *width, int *height);
extern void microTextureFree(int textureId);
extern void microTexttureSetFilter(int textureId, int filter);


//view
extern void microViewSet(
    float viewportX, float viewportY,
    float viewportWidth, float viewportHeight,
    float centerX, float centerY, float width,
    float height, float rotation, int flipY);
extern void microViewUpdate();
extern void microViewFlipY(int flipY);
extern void microViewSetViewport(float x, float y, float width, float height);
extern void microViewSetCenter(float x, float y);
extern void microViewSetSize(float width, float height);
extern void microViewSetRotation(float rotation);
extern void microViewGetCenter(float *centerX, float *centerY);
extern void microViewGetSize(float *width, float *height);
extern float microViewGetRotation();


//shaders
extern int microShaderLoadFromFile(const char *vertexShaderPath, const char *fragmentShaderPath);
extern int microShaderLoadFromSource(const char *vertexShaderSrc, const char *fragmentShaderSrc);
extern void microShaderFree(int shaderId);
extern void microShaderApply(int shaderId);
extern void microShaderSetUniform1f(const char *name, float value);
extern void microShaderSetUniform2f(const char *name, float value1, float value2);
extern void microShaderSetUniform3f(const char *name, float value1, float value2, float value3);
extern void microShaderSetUniform4f(const char *name, float value1, float value2, float value3, float value4);
extern void microShaderSetUniform1i(const char *name, int value);
extern void microShaderSetUniform2i(const char *name, int value1, int value2);
extern void microShaderSetUniform3i(const char *name, int value1, int value2, int value3);
extern void microShaderSetUniform4i(const char *name, int value1, int value2, int value3, int value4);
extern void microShaderSetMatrix4(const char *name, float *matrix);


//canvas
extern int microCanvasCreate(int width, int height);
extern int microCanvasGetTextureId(int canvasId);
extern void microCanvasFree(int canvasId);

//rendering
extern void microGraphicsInit(SDL_Window *window);
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

#endif
