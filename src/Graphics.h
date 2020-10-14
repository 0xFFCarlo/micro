#ifndef GRAPHICS_H
#define GRAPHICS_H

//texture
extern int microTextureLoadFromFile(const char *filepath);
extern void microTextureGetSize(int textureId, int *width, int *height);
extern void microTextureFree(int textureId);

//view
extern void microViewSet(float viewportX, float viewportY, float viewportWidth, float viewportHeight,
		float centerX, float centerY, float width, float height, float rotation);
extern void microViewUpdate();
extern void microViewSetViewport(float x, float y, float width, float height);
extern void microViewSetCenter(float x, float y);
extern void microViewSetSize(float width, float height);
extern void microViewSetRotation(float rotation);
extern void microViewGetCenter(float *centerX, float *centerY);
extern void microViewGetSize(float *width, float *height);
extern float microViewGetRotation();

//rendering
extern void microGraphicsInit();
extern void microGraphicsQuit();
extern void microGraphicsClear();
extern void microGraphicsDisplay();
extern void microGraphicsDrawRect(int textureId, float tx, float ty, float tw, float th,
		float x, float y, float w, float h, unsigned long color);
extern void microGraphicsDrawRectRot(int textureId, float tx, float ty, float tw, float th,
		float x, float y, float w, float h, float originX, float originY,
		float rotation, unsigned long color);

#endif
