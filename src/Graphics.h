#ifndef GRAPHICS_H
#define GRAPHICS_H

//texture
extern int microLoadTextureFromFile(const char *filepath);
extern void microGetTextureSize(int textureId, int *width, int *height);
extern void microUnloadTexture(int textureId);

//view
extern void microSetView(float viewportX, float viewportY, float viewportWidth, float viewportHeight,
							float centerX, float centerY, float width, float height, float rotation);
extern void microUpdateView();
extern void microSetViewViewport(float x, float y, float width, float height);
extern void microSetViewCenter(float x, float y);
extern void microSetViewSize(float width, float height);
extern void microSetViewRotation(float rotation);
extern void microGetViewCenter(float *centerX, float *centerY);
extern void microGetViewSize(float *width, float *height);
extern float microGetViewRotation();

//rendering
extern void microGraphicsInit();
extern void microGraphicsQuit();
extern void microClear();
extern void microDisplay();
extern void microDrawRect(int textureId, float tx, float ty, float tw, float th,
						  float x, float y, float w, float h, unsigned long color);
extern void microDrawRectWithRotation(int textureId, float tx, float ty, float tw, float th,
									  float x, float y, float w, float h, float originX, float originY,
									  float rotation, unsigned long color);

#endif