#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengles.h>
#include "Graphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MICRO_MAX_TEXTURES 64
#define MICRO_VERTEX_BUFFER_SIZE 1024
#define MICRO_INDICES_BUFFER_SIZE 1536 //vertexBufSize * 1.5

//GL states
static unsigned int currentTexture = -1;
static unsigned int currentArrayBuffer;

//texture buffers
typedef struct microTexture
{
	GLuint id;
	int width, height, channels;
} microTexture;
static microTexture* loadedTextures[MICRO_MAX_TEXTURES];
static unsigned char cleanedBuffer = GL_FALSE;

//renderer buffers
static float vertexbuf[MICRO_VERTEX_BUFFER_SIZE][2];
static float texsourcebuf[MICRO_VERTEX_BUFFER_SIZE][2];
static unsigned char colorbuf[MICRO_VERTEX_BUFFER_SIZE][4];
static unsigned int ibo;
static int buffersize;
static int countverts;
static int wireframe;

//view infos
float viewViewportX, viewViewportY, viewViewportW, viewViewportH;
float viewCenterX, viewCenterY;
float viewWidth, viewHeight;
float viewRotation;
float viewMatrix[16];

////////////////////////////
//GL STATES
////////////////////////////
void microGLStateBindTexture(int textureId)
{
	if (currentTexture == textureId) return;
	glBindTexture(GL_TEXTURE_2D, textureId);
	currentTexture = textureId;
}

void microGLStateBindBuffer(unsigned int id, unsigned int type)
{
	if (currentArrayBuffer == id) return;
	glBindBuffer(type, id);
	currentArrayBuffer = id;
}

void microGLStateReset()
{
	microGLStateBindTexture(0);
	microGLStateBindBuffer(0, GL_ARRAY_BUFFER);
}

////////////////////////////
//TEXTURE
////////////////////////////
int microTextureLoadFromFile(const char* filepath)
{
	//clean resources uffer the first time
	if (cleanedBuffer == GL_FALSE) {
		memset(loadedTextures, 0, sizeof(microTexture*));
		cleanedBuffer = GL_TRUE;
	}

	//Variables
	int stbi_fmt = STBI_rgb_alpha;
	int fsize = 0;

	//Open file
	FILE* file = fopen(filepath, "rb");

	if (!file) {
		printf("Error: can't open the file\n");
		fclose(file);
		return -1;
	}

	//Get file size
	fseek(file, 0L, SEEK_END);
	fsize = (int)ftell(file);

	//Seek to the beginning
	fseek(file, 0L, SEEK_SET);

	//Read all the file into filedata
	stbi_uc* fileData = (stbi_uc*)malloc(sizeof(stbi_uc) * fsize);
	fread(fileData, sizeof(stbi_uc) * fsize, 1, file);
	fclose(file);

	//Get the image data
	int width;
	int height;
	int channels;
	unsigned char* data;
	data = stbi_load_from_memory(fileData, fsize, &width, &height, &channels, stbi_fmt);

	//Free the filedata
	free(fileData);

	//Check for stb_image fail
	if (data == NULL) {
		printf("Error: can't generate texture\n");
		printf("Details : %s\n", stbi_failure_reason());
		return -1;
	}

	//Get the texture format
	unsigned int fmt = GL_RGBA;
	switch (channels) {
		case 1: fmt = GL_ALPHA; break;
		case 3: fmt = GL_RGB; break;
		case 4: fmt = GL_RGBA; break;
		default:
				  printf("Error: data file format [%d] not recognized\n", fmt);
				  return -1;
	}

	//Create texture
	GLuint id;
	glGenTextures(1, &id);

	microGLStateBindTexture(id);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt,
			GL_UNSIGNED_BYTE, data);

	//not needed anymore
	free(data);

	//Check for opengl errors
	GLenum glerr = glGetError();
	if (glerr != GL_NO_ERROR) {
		printf("Error: opengl error %d\n", glerr);
	}

	//find spot in the resources buffer
	microTexture* texture = malloc(sizeof(microTexture));
	texture->id = id;
	texture->width = width;
	texture->height = height;
	texture->channels = channels;
	int spot = -1;
	for (int i = 0; i < MICRO_MAX_TEXTURES; i++) {
		if (loadedTextures[i] == NULL) {
			spot = i;
			loadedTextures[i] = texture;
		}
	}

	return spot;    
}

void microTextureGetSize(int textureId, int *width, int *height)
{
	if (width != NULL) *width = loadedTextures[textureId]->width;
	if (height != NULL) *height = loadedTextures[textureId]->height;
}

void microTextureFree(int textureId)
{
	if (loadedTextures[textureId] != NULL) {
		glDeleteTextures(1, &loadedTextures[textureId]->id);
		free(loadedTextures[textureId]);
		loadedTextures[textureId] = NULL;
	}
}

////////////////////////////
// RENDERING
///////////////////////////
void microGraphicsInit()
{
	//Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//Init opengl
	//Enable textures
	glEnable(GL_TEXTURE_2D);
	assert(glGetError() == GL_NO_ERROR);

	//Enable Alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	assert(glGetError() == GL_NO_ERROR);

	//Enable vertex array
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	assert(glGetError() == GL_NO_ERROR);

	//Reset glstate
	microGLStateReset();

	//Setup renderer
	buffersize = 0;
	countverts = 0;
	wireframe = 0;

	//Generate indices buffer object
	glGenBuffers(1, &ibo);

	//Calculate indices data
	const int buffersize = MICRO_INDICES_BUFFER_SIZE;
	unsigned short indsbuf[buffersize];
	unsigned short ind = 0;
	int i;
	for (i = 0; i < buffersize - 5; i+=6)
	{
		indsbuf[i] = ind;
		indsbuf[i + 1] = ind + 1;
		indsbuf[i + 2] = ind + 2;
		indsbuf[i + 3] = ind + 2;
		indsbuf[i + 4] = ind + 3;
		indsbuf[i + 5] = ind;
		ind += 4;	
	}

	//Send indices data to the graphics card
	microGLStateBindBuffer(ibo, GL_ELEMENT_ARRAY_BUFFER);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * buffersize,
			indsbuf, GL_STATIC_DRAW);
	assert(glGetError() == GL_NO_ERROR);

	//We don't need ArrayBuffer
	microGLStateBindBuffer(0, GL_ARRAY_BUFFER);
}

void microGraphicsQuit()
{
	//Disable opengl stuff
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//Delete indices buffer object
	microGLStateBindBuffer(ibo, GL_ELEMENT_ARRAY_BUFFER);
	glDeleteBuffers(1, &ibo);
}

void microGraphicsClear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void microGraphicsDisplay()
{
	if (!countverts) return;

	//Draw
	glVertexPointer(2, GL_FLOAT, 0, vertexbuf);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorbuf);
	glTexCoordPointer(2, GL_FLOAT, 0, texsourcebuf);
	microGLStateBindBuffer(ibo, GL_ELEMENT_ARRAY_BUFFER);
	glDrawElements(GL_TRIANGLES, countverts, GL_UNSIGNED_SHORT, 0);
	assert(glGetError() == GL_NO_ERROR);

	buffersize = 0;
	countverts = 0;
}

void microGraphicsDraw(int textureId, float x, float y, float x2, float y2,
		float x3, float y3, float x4, float y4, float tx_x, float tx_y,
		float tx_w, float tx_h, unsigned long color)
{
	if (textureId == 0)
	{
		if (currentTexture != 0) {
			microGraphicsDisplay();
			microGLStateBindTexture(0);
		}
	}
	else
	{
		if (currentTexture != textureId) {
			microGraphicsDisplay();
			microGLStateBindTexture(textureId);
		}
	}

	if (buffersize == 1024)
		microGraphicsDisplay();

	//Vertex 1
	vertexbuf[buffersize][0] = x;
	vertexbuf[buffersize][1] = y;

	texsourcebuf[buffersize][0] = tx_x;
	texsourcebuf[buffersize][1] = tx_y;

	colorbuf[buffersize][0] = (color & 0xFF000000) >> 24;
	colorbuf[buffersize][1] = (color & 0x00FF0000) >> 16;
	colorbuf[buffersize][2] = (color & 0x0000FF00) >> 8;
	colorbuf[buffersize][3] = color & 0x000000FF;

	buffersize++;

	//Vertex 2
	vertexbuf[buffersize][0] = x2;
	vertexbuf[buffersize][1] = y2;

	texsourcebuf[buffersize][0] = tx_x + tx_w;
	texsourcebuf[buffersize][1] = tx_y;

	colorbuf[buffersize][0] = (color & 0xFF000000) >> 24;
	colorbuf[buffersize][1] = (color & 0x00FF0000) >> 16;
	colorbuf[buffersize][2] = (color & 0x0000FF00) >> 8;
	colorbuf[buffersize][3] = color & 0x000000FF;

	buffersize++;

	//Vertex 3
	vertexbuf[buffersize][0] = x3;
	vertexbuf[buffersize][1] = y3;

	texsourcebuf[buffersize][0] = tx_x + tx_w;
	texsourcebuf[buffersize][1] = tx_y + tx_h;

	colorbuf[buffersize][0] = (color & 0xFF000000) >> 24;
	colorbuf[buffersize][1] = (color & 0x00FF0000) >> 16;
	colorbuf[buffersize][2] = (color & 0x0000FF00) >> 8;
	colorbuf[buffersize][3] = color & 0x000000FF;

	buffersize++;

	//Vertex 4
	vertexbuf[buffersize][0] = x4;
	vertexbuf[buffersize][1] = y4;

	texsourcebuf[buffersize][0] = tx_x;
	texsourcebuf[buffersize][1] = tx_y + tx_h;

	colorbuf[buffersize][0] = (color & 0xFF000000) >> 24;
	colorbuf[buffersize][1] = (color & 0x00FF0000) >> 16;
	colorbuf[buffersize][2] = (color & 0x0000FF00) >> 8;
	colorbuf[buffersize][3] = color & 0x000000FF;

	buffersize++;

	countverts += 6;
}

void microGraphicsDrawRectRot(int textureId, float tx, float ty, float tw, float th,
		float x, float y, float w, float h, float originX, float originY,
		float rotation, unsigned long color)
{
	static float v1X, v1Y;
	static float v2X, v2Y;
	static float v3X, v3Y;
	static float v4X, v4Y;


	if (rotation)
	{
		const float ca = cosf(rotation * 3.14159 / 180);
		const float sa = sinf(rotation * 3.14159 / 180);

		const float cx = x + originX;
		const float cy = y + originY;

		const float ox = x - cx;
		const float oy = y - cy;
		const float ow = x + w - cx;
		const float oh = y + h - cy;

		v1X = ox*ca - oy*sa + cx;
		v1Y = ox*sa + oy*ca + cy;

		v2X = ow*ca - oy*sa + cx;
		v2Y = ow*sa + oy*ca + cy;

		v3X = ow*ca - oh*sa + cx;
		v3Y = ow*sa + oh*ca + cy;

		v4X = ox*ca - oh*sa + cx;
		v4Y = ox*sa + oh*ca + cy;
	}
	else
	{
		v1X = x - originX;
		v1Y = y - originY;

		v2X = x + w - originX;
		v2Y = y - originY;

		v3X = x + w - originX;
		v3Y = y + h - originY;

		v4X = x - originX;
		v4Y = y + h - originY;
	}

	if (textureId != -1)
	{
		static int tex_w, tex_h;
		static int glTextureId;
		assert(loadedTextures[textureId] != NULL);
		microTextureGetSize(textureId, &tex_w, &tex_h);
		glTextureId = loadedTextures[textureId]->id;

		microGraphicsDraw(glTextureId,
				v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y,
				tx / tex_w,
				ty / tex_h,
				tw / tex_w,
				th / tex_h,
				color);
	}
	else
	{
		microGraphicsDraw(0,
				v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y,
				0, 0, 0, 0, color);
	}
}

void microGraphicsDrawRect(int textureId, float tx, float ty, float tw, float th,
		float x, float y, float w, float h, unsigned long color)
{
	if (textureId == -1) 
	{
		microGraphicsDraw(0, x, y, x + w, y, x + w, y + h, x, y + h, 0, 0, 0, 0, color);
	}
	else
	{
		static int tex_w, tex_h;
		static int glTextureId;
		assert(loadedTextures[textureId] != NULL);
		microTextureGetSize(textureId, &tex_w, &tex_h);
		glTextureId = loadedTextures[textureId]->id;
		microGraphicsDraw(glTextureId, x, y, x + w, y, x + w, y + h, x, y + h,
				tx / (float)tex_w, ty / (float)tex_h, tw / (float)tex_w,
				th / (float)tex_h, color);
	}
}

//VIEW
void microViewSet(float viewportX, float viewportY, float viewportWidth, float viewportHeight,
		float centerX, float centerY, float width, float height, float rotation)
{
	viewViewportX = viewportX;
	viewViewportY = viewportY;
	viewViewportW = viewportWidth;
	viewViewportH = viewportHeight;
	viewCenterX = centerX;
	viewCenterY = centerY;
	viewWidth = width;
	viewHeight = height;
	viewRotation = rotation;
}

void microViewUpdate()
{
	float NCenterX, NCenterY;
	NCenterX = floor(viewCenterX);
	NCenterY = floor(viewCenterY);

	// Rotation components
	float angle = viewRotation * 3.141592654f / 180.f;
	float cosine = cosf(angle);
	float sine = sinf(angle);
	float tx = -NCenterX * cosine - NCenterY * sine + NCenterX;
	float ty = NCenterX * sine - NCenterY * cosine + NCenterY;

	// Projection components
	float a = 2.f / viewWidth;
	float b = -2.f / viewHeight;
	float c = -a * NCenterX;
	float d = -b * NCenterY;

	const float zNear = 0;
	const float zFar = 10;
	const float e = 2.f / (zFar - zNear);
	const float f = -(zFar + zNear) / (zFar - zNear);

	//Update matrix
	viewMatrix[0] = a * cosine;
	viewMatrix[1] = -b * sine;
	viewMatrix[2] = 0.f;
	viewMatrix[3] = 0.f;
	viewMatrix[4] = a * sine;
	viewMatrix[5] = b * cosine;
	viewMatrix[6] = 0.f;
	viewMatrix[7] = 0.f;
	viewMatrix[8] = 0.f;
	viewMatrix[9] = 0.f;
	viewMatrix[10] = e;
	viewMatrix[11] = 0.f;
	viewMatrix[12] = a * tx + c;
	viewMatrix[13] = b * ty + d;
	viewMatrix[14] = f;
	viewMatrix[15] = 1.f;

	//Set viewport
	glViewport(viewViewportX, viewViewportY, viewViewportW, viewViewportH);

	//Apply view
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(viewMatrix);
	glMatrixMode(GL_MODELVIEW);
}

void microViewSetViewport(float x, float y, float width, float height)
{
	viewViewportX = x;
	viewViewportY = y;
	viewViewportW = width;
	viewViewportH = height;
}

void microViewSetCenter(float x, float y)
{
	viewCenterX = x;
	viewCenterY = y;
}

void microViewSetSize(float width, float height)
{
	viewWidth = width;
	viewHeight = height;
}

void microViewSetRotation(float rotation)
{
	viewRotation = rotation;
}

void microViewGetCenter(float* centerX, float* centerY)
{
	*centerX = viewCenterX;
	*centerY = viewCenterY;
}

void microViewGetSize(float* width, float* height)
{
	*width = viewWidth;
	*height = viewHeight;
}

float microViewGetRotation()
{
	return viewRotation;
}
