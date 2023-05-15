#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#if defined(__linux__)
//#include <GLES3/gl3.h>
//#include <GL/glu.h>
#include <GL/glew.h>
#include <GL/gl.h>
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#include <math.h>

#include "Graphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MICRO_MAX_TEXTURES 64
#define MICRO_MAX_CANVASES 64
#define MICRO_MAX_SHADERS 64
#define MICRO_MAX_ATTRIBUTES 16
#define MICRO_MAX_UNIFORMS 16
#define MICRO_MAX_NAME_LEN 32
#define MICRO_VERTEX_BUFFER_SIZE (6 * 256)
#define MICRO_MAX_SHADER_LEN 8000

//GL states
static int currentTexture = -1;
static int currentShader = -1;

//texture buffers
typedef struct microTexture
{
  GLuint id;
  int width, height, channels;
} microTexture;
static microTexture microTextures[MICRO_MAX_TEXTURES];
static unsigned char cleanedBuffer = GL_FALSE;

typedef struct microShader
{
  GLuint programId;
  char uniformsNames[MICRO_MAX_UNIFORMS][MICRO_MAX_NAME_LEN];
  int uniformsLocations[MICRO_MAX_UNIFORMS];
  GLenum uniformsTypes[MICRO_MAX_UNIFORMS];
  double uniformsValues[MICRO_MAX_UNIFORMS * 4];
  int uniformsCount;
} microShader;
static microShader microShaders[MICRO_MAX_SHADERS];

typedef struct microCanvas
{
  GLuint framebufferId;
  int microTextureId;
  int width, height;
} microCanvas;
static microCanvas microCanvases[MICRO_MAX_CANVASES];

//shader stuff
static const char baseVertexShaderSrc[] = "#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texcoord;\n"
"layout (location = 2) in vec4 color;\n"
"out vec4 o_color;\n"
"out vec2 o_texcoord;\n"
"uniform mat4 u_view;\n"
"void main()\n"
"{\n"
"  gl_Position = u_view * vec4(position, 0.0, 1.0);\n"
"  o_color = color;\n"
"  o_texcoord = texcoord;\n"
"}\n";

static const char baseFragmentShaderSrc[] = "#version 330 core\n"
"in vec4 o_color;\n"
"in vec2 o_texcoord;\n"
"out vec4 fragColor;\n"
"uniform sampler2D u_texture;\n"
"void main()\n"
"{\n"
" //texture + alpha blending\n"
"  vec4 texColor = texture(u_texture, o_texcoord);\n"
"  fragColor = texColor * o_color;\n"
"}\n";

static int defaultShaderId = -1;



//renderer buffers
static SDL_GLContext* context = NULL;
static SDL_Window* window = NULL;
static float vertexbuf[MICRO_VERTEX_BUFFER_SIZE][2];
static float texsourcebuf[MICRO_VERTEX_BUFFER_SIZE][2];
static float colorbuf[MICRO_VERTEX_BUFFER_SIZE][4];
static unsigned int vao;
static unsigned int vbo, cbo, tbo;
static int countverts;
static int wireframe;

//view infos
float viewViewportX, viewViewportY, viewViewportW, viewViewportH;
float viewCenterX, viewCenterY;
float viewWidth, viewHeight;
float viewRotation;
float viewMatrix[16];
int viewFlipY = 0;
int viewUpdated = 0;

////////////////////////////
//GL STATES
////////////////////////////
void microGLStateBindTexture(int textureId)
{
  if (currentTexture == textureId) return;
  glBindTexture(GL_TEXTURE_2D, textureId);
  currentTexture = textureId;
}

void microGLStateReset()
{
  microGLStateBindTexture(0);
}

void microGLCheckErrors()
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
    printf("OpenGL Error: %d\n", err);
}

////////////////////////////
//TEXTURE
////////////////////////////
int microTextureLoadFromFile(const char* filepath)
{
  //clean resources uffer the first time
  if (cleanedBuffer == GL_FALSE) {
    memset(microTextures, 0, sizeof(microTexture*));
    cleanedBuffer = GL_TRUE;
  }

  //Variables
  int stbi_fmt = STBI_rgb_alpha;
  int fsize = 0;

  //Open file
  FILE* file = fopen(filepath, "rb");

  if (!file) {
    printf("Error: can't open the file %s\n", filepath);
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
  channels = 4;

  //Free the filedata
  free(fileData);

  //Check for stb_image fail
  if (data == NULL) {
    printf("Error: can't generate texture\n");
    printf("Details : %s\n", stbi_failure_reason());
    return -1;
  }
  
  return microTextureLoadFromMemory(data, width, height, channels, GL_LINEAR);
}

int microTextureLoadFromMemory(const unsigned char *data, const unsigned int width, const unsigned int height, const unsigned int channels, const unsigned int filter)
{
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
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt,
      GL_UNSIGNED_BYTE, data);

  //Check for opengl errors
  microGLCheckErrors();

  //find spot in the resources buffer
  microTexture texture;
  texture.id = id;
  texture.width = width;
  texture.height = height;
  texture.channels = channels;
  int spot = -1;
  for (int i = 0; i < MICRO_MAX_TEXTURES; i++) {
    if (microTextures[i].id == -1) {
      spot = i;
      microTextures[i] = texture;
      break;
    }
  }
  assert(spot != -1);

  return spot;    
}

void microTexttureSetFilter(int textureId, int filter)
{
  microGLStateBindTexture(microTextures[textureId].id);
  if (filter == MICRO_FILTER_LINEAR) {
    filter = GL_LINEAR;
  } else if (filter == MICRO_FILTER_NEAREST) {
    filter = GL_NEAREST;
  } else {
    printf("Error: filter %d not recognized\n", filter);
    return;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}


void microTextureGetSize(int textureId, int *width, int *height)
{
  if (width != NULL) *width = microTextures[textureId].width;
  if (height != NULL) *height = microTextures[textureId].height;
}

void microTextureFree(int textureId)
{
  if (microTextures[textureId].id != -1) {
    glDeleteTextures(1, &microTextures[textureId].id);
    microTextures[textureId].id = -1;
  }
}



////////////////////////////
//SHADER
////////////////////////////
void readShaderFile(const char* filepath, char* dst)
{
  FILE* file = fopen(filepath, "rb");
  if (!file) {
    printf("Error: can't open the file %s\n", filepath);
    fclose(file);
    return;
  }

  //Get file size
  fseek(file, 0L, SEEK_END);
  int fsize = (int)ftell(file);
  assert(fsize < MICRO_MAX_SHADER_LEN);

  //Seek to the beginning
  fseek(file, 0L, SEEK_SET);

  //Read all the file into filedata
  fread(dst, sizeof(char) * fsize, 1, file);
  dst[fsize] = '\0';
  fclose(file);
}

int microShaderLoadFromFile(const char *vertexShaderPath, const char *fragmentShaderPath)
{
  char vertShaderSrc[MICRO_MAX_SHADER_LEN];
  char fragShaderSrc[MICRO_MAX_SHADER_LEN];
  readShaderFile(vertexShaderPath, &vertShaderSrc[0]);
  readShaderFile(fragmentShaderPath, &fragShaderSrc[0]);
  return microShaderLoadFromSource(&vertShaderSrc[0], &fragShaderSrc[0]);
}

int microShaderLoadFromSource(const char *vertexShaderSrc, const char *fragmentShaderSrc)
{
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

  GLint result = GL_FALSE;
  int logLength;

  // Compile vertex shader

  const char* vertShaderSrcPtr = &vertexShaderSrc[0];
  glShaderSource(vertShader, 1, &vertShaderSrcPtr, NULL);
  glCompileShader(vertShader);

  char errorLog[4096];

  // Check vertex shader
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
  glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
  assert(logLength < 4096);
  glGetShaderInfoLog(vertShader, logLength, NULL, &errorLog[0]);
  if (logLength != 0) {
    printf("Error compiling vertex shader\n");
    printf("%s\n", &errorLog[0]);
  }

  // Compile fragment shader
  
  const char* fragShaderSrcPtr = &fragmentShaderSrc[0];
  glShaderSource(fragShader, 1, &fragShaderSrcPtr, NULL);
  glCompileShader(fragShader);

  // Check fragment shader

  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
  glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
  assert(logLength < 4096);
  glGetShaderInfoLog(fragShader, logLength, NULL, &errorLog[0]);
  if (logLength != 0) {
    printf("Error compiling fragment shader\n");
    printf("%s\n", &errorLog[0]);
  }

  int program = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &result);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
  assert(logLength < 4096);
  glGetProgramInfoLog(program, logLength, NULL, &errorLog[0]);
  if (logLength != 0)
    printf("%s\n", &errorLog[0]);

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  microShader shader;
  shader.programId = program;
  assert(shader.programId != -1);

  //Load uniforms
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &shader.uniformsCount);
  for (int i = 0; i < shader.uniformsCount; i++)
  {
    GLsizei length;
    GLint size;
    glGetActiveUniform(program, (GLuint)i, MICRO_MAX_NAME_LEN, &length, &size, &shader.uniformsTypes[i], shader.uniformsNames[i]);
    shader.uniformsLocations[i] = glGetUniformLocation(program, shader.uniformsNames[i]);
    shader.uniformsValues[i] = 0.0;
  }
  
  //Find a spot to store the shader
  int spot = -1;
  for (int i = 0; i < MICRO_MAX_SHADERS; i++) {
    if (microShaders[i].programId == -1) {
      spot = i;
      microShaders[i] = shader;
      break;
    }
  }
  assert(spot != -1);

  return spot;
}

int microShaderGetProgramID(int shaderId)
{
  assert(shaderId != -1);
  return microShaders[shaderId].programId;
}

int microShaderGetCurrent()
{
  return currentShader;
}

void microShaderFree(int shaderId)
{
  if (shaderId == -1 || microShaders[shaderId].programId == -1) return;
  if (currentShader == shaderId) currentShader = -1;
  glDeleteProgram(microShaders[shaderId].programId);
  microShaders[shaderId].programId = -1;
}

void microShaderApply(int shaderId)
{
  if (currentShader == shaderId) return;
  glUseProgram(microShaders[shaderId].programId);
  currentShader = shaderId;
}

void microShaderSetUniform(const char *name, ...)
{
  int uniformLoc = -1;
  int uniformId = -1;
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      uniformId = i;
      uniformLoc = shader->uniformsLocations[i];

    }
  }
  assert(uniformId != -1); //Uniform not found
  
  va_list args;
  va_start(args, name);
  switch (shader->uniformsTypes[uniformId]) {
    case GL_FLOAT:
      shader->uniformsValues[uniformId * 4] = (double)va_arg(args, double);
      glUniform1f(uniformLoc, shader->uniformsValues[uniformId * 4]);
      break;
    case GL_FLOAT_VEC2:
      shader->uniformsValues[uniformId * 4] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 1] = (double)va_arg(args, double);
      glUniform2f(uniformLoc, shader->uniformsValues[uniformId * 4],
          shader->uniformsValues[uniformId * 4 + 1]);
      break;
    case GL_FLOAT_VEC3:
      shader->uniformsValues[uniformId * 4] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 1] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 2] = (double)va_arg(args, double);
      glUniform3f(uniformLoc, shader->uniformsValues[uniformId * 4],
          shader->uniformsValues[uniformId * 4 + 1],
          shader->uniformsValues[uniformId * 4 + 2]);
      break;
    case GL_FLOAT_VEC4:
      shader->uniformsValues[uniformId * 4] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 1] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 2] = (double)va_arg(args, double);
      shader->uniformsValues[uniformId * 4 + 3] = (double)va_arg(args, double);
      glUniform4f(uniformLoc, shader->uniformsValues[uniformId * 4],
          shader->uniformsValues[uniformId * 4 + 1],
          shader->uniformsValues[uniformId * 4 + 2],
          shader->uniformsValues[uniformId * 4 + 3]);
      break;
    case GL_FLOAT_MAT4:
      assert(0); //Not implemented
      break;
    default:
      assert(0); //Unsupported uniform type
  }
}

void microShaderGetUniform1(const char *name, double* v1)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      *v1 = shader->uniformsValues[i * 4];
      return;
    }
  }
  assert(0); //Uniform not found
}

void microShaderGetUniform2(const char *name, double* v1, double* v2)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      return;
    }
  }
  assert(0); //Uniform not found
}

void microShaderGetUniform3(const char *name, double* v1, double* v2, double* v3)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      *v3 = shader->uniformsValues[i * 4 + 2];
      return;
    }
  }
  assert(0); //Uniform not found
}

void microShaderGetUniform4(const char *name, double* v1, double* v2, double* v3, double* v4)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      *v3 = shader->uniformsValues[i * 4 + 2];
      *v4 = shader->uniformsValues[i * 4 + 3];
      return;
    }
  }
  assert(0); //Uniform not found
}

void microShaderSetMatrix4(const char *name, float *matrix)
{
  int uniformLoc = -1;
  int uniformId = -1;
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++) {
    if (strcmp(shader->uniformsNames[i], name) == 0) {
      uniformId = i;
      uniformLoc = shader->uniformsLocations[i];

    }
  }
  assert(uniformId != -1); //Uniform not found
  glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, matrix);
}



////////////////////////////
/// CANVAS
///////////////////////////
int microCanvasCreate(int width, int height)
{
  //create framebuffer
  GLuint framebufferId = 0;
  glGenFramebuffers(1, &framebufferId);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
  glViewport(0, 0, width, height);
  assert(framebufferId != 0);

  
  //create black texture
  const int textureId = microTextureLoadFromMemory(0, width, height, 4, GL_NEAREST);
  assert(textureId != -1);
  assert(microTextures[textureId].id != -1);
  microGLCheckErrors();

  //find spot in the resources buffer
  microCanvas canvas;
  canvas.framebufferId = framebufferId;
  canvas.microTextureId = textureId;
  canvas.width = width;
  canvas.height = height;
  int canvasId = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++) {
    if (microCanvases[i].framebufferId == -1) {
      canvasId = i;
      microCanvases[i] = canvas;
      break;
    }
  }
  assert(canvasId != -1);
  
  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, microTextures[textureId].id, 0);
  microGLCheckErrors();

  // Set the list of draw buffers.
  //GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  //glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
  microGLCheckErrors();
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Error creating framebuffer\n");
    return -1;
  }

  return canvasId;
}

int microCanvasGetTextureId(int canvasId)
{
  return microCanvases[canvasId].microTextureId;
}

void microCanvasFree(int canvasId)
{
  glDeleteFramebuffers(1, &microCanvases[canvasId].framebufferId);
  microCanvases[canvasId].framebufferId = -1;
  microTextureFree(microCanvases[canvasId].microTextureId);
}


////////////////////////////
// RENDERING
///////////////////////////
void microGraphicsInit()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize th SDL2 library\n");
    return;
  }
  //create window and opengl context
  window = SDL_CreateWindow("micro",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800, 600,
      SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
      );

  if (!window) {
    printf("Failed to create the window\n");
    return;
  }

  //clean textures and canvases
  for (int i = 0; i < MICRO_MAX_TEXTURES; i++)
    microTextures[i].id = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++)
    microCanvases[i].framebufferId = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++)
    microShaders[i].programId = -1;

	//Set OpenGL version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	//Initilize Opengl
	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		printf("Couldn't create OpenGL context\n");
	}

  // Set VSYNC, try adaptive first and if not supported, use normal vsync
	if (SDL_GL_SetSwapInterval(-1) == -1)
    SDL_GL_SetSwapInterval(1);

	char *glVersion = (char*)glGetString(GL_VERSION);
	if (glVersion) {
		printf("OpenGL version: %s\n", glVersion);
	}

#if defined(__linux__)
	GLenum err = glewInit();
	if (err != GLEW_OK)
		exit(1); 
	else
		printf("GLEW version: %s\n", glewGetString(GLEW_VERSION));
#endif

  //Load base shader
  defaultShaderId = microShaderLoadFromSource(baseVertexShaderSrc, baseFragmentShaderSrc);
  int programId = microShaderGetProgramID(defaultShaderId);
  assert(defaultShaderId != -1);
  microShaderApply(defaultShaderId);
  const int positionLoc = glGetAttribLocation(programId, "position");
  const int texCoordLoc = glGetAttribLocation(programId, "texcoord");
  const int colorLoc = glGetAttribLocation(programId, "color");

  //Clear
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0, 0.0, 0.0, 0.0);

  //Enable Alpha
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  microGLCheckErrors();

  //create VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
  microGLCheckErrors();

	//Create vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 2, &vertexbuf[0], GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionLoc);
  microGLCheckErrors();
	
  //Create texture coordinates buffer
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 2, &texsourcebuf[0], GL_STATIC_DRAW);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texCoordLoc);
  microGLCheckErrors();
  
  //Create color buffer
	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 4, &colorbuf[0], GL_STATIC_DRAW);
	glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colorLoc);
  microGLCheckErrors();

  //Setup renderer
  countverts = 0;
  wireframe = 0;

  printf("microGraphics allocated %d kb\n", (int)((sizeof(microTextures) + sizeof(microCanvases) + sizeof(microShaders)) / (double)(1024)));
}

void microGraphicsQuit()
{
  //Delete context 
  SDL_GL_DeleteContext( context );
  SDL_DestroyWindow( window );
  SDL_Quit();
}

void microGraphicsClear()
{
  glClear(GL_COLOR_BUFFER_BIT);
}

void microGraphicsRenderToScreen()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int windowWidth, windowHeight;
  SDL_GetWindowSize(window, &windowWidth, &windowHeight);
  glViewport(0, 0, windowWidth, windowHeight);
  microGLCheckErrors();
}

void microGraphicsRenderToCanvas(int canvasId)
{
  assert(canvasId >= 0 && canvasId < MICRO_MAX_CANVASES);
  glBindFramebuffer(GL_FRAMEBUFFER, microCanvases[canvasId].framebufferId);
  glViewport(0, 0, microCanvases[canvasId].width, microCanvases[canvasId].height);
  microGLCheckErrors();
}

void microGraphicsDisplay()
{
  if (!countverts) return;
	
  //Create vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 2, &vertexbuf[0], GL_STATIC_DRAW);
	
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 2, &texsourcebuf[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 4, &colorbuf[0], GL_STATIC_DRAW);

	glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, countverts); 
  microGLCheckErrors();

  countverts = 0;
}

void microGraphicsDraw(int textureId, float x, float y, float x2, float y2,
    float x3, float y3, float x4, float y4, float tx_x, float tx_y,
    float tx_w, float tx_h, float r, float g, float b, float a)
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

  if (countverts == MICRO_VERTEX_BUFFER_SIZE)
    microGraphicsDisplay();

  unsigned int buffersize = countverts;

  //V1
  vertexbuf[buffersize][0] = x;
  vertexbuf[buffersize][1] = y;

  texsourcebuf[buffersize][0] = tx_x;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  //V2
  vertexbuf[buffersize][0] = x2;
  vertexbuf[buffersize][1] = y2;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;
  
  //V3
  vertexbuf[buffersize][0] = x4;
  vertexbuf[buffersize][1] = y4;

  texsourcebuf[buffersize][0] = tx_x;
  texsourcebuf[buffersize][1] = tx_y + tx_h;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;
  
  buffersize++;
  
  //V4
  vertexbuf[buffersize][0] = x2;
  vertexbuf[buffersize][1] = y2;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;
  
  buffersize++;

  //V5
  vertexbuf[buffersize][0] = x3;
  vertexbuf[buffersize][1] = y3;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y + tx_h;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  //V6
  vertexbuf[buffersize][0] = x4;
  vertexbuf[buffersize][1] = y4;

  texsourcebuf[buffersize][0] = tx_x;
  texsourcebuf[buffersize][1] = tx_y + tx_h;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  countverts += 6;
}

void microGraphicsDrawRectRot(int textureId, float tx, float ty, float tw, float th,
    float x, float y, float w, float h, float originX, float originY,
    float rotation, float r, float g, float b, float a)
{
  static float v1X, v1Y;
  static float v2X, v2Y;
  static float v3X, v3Y;
  static float v4X, v4Y;


  if (rotation)
  {
    const float ca = cosf(rotation * M_PI / 180.0);
    const float sa = sinf(rotation * M_PI / 180.0);

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
    assert(microTextures[textureId].id != -1);
    microTextureGetSize(textureId, &tex_w, &tex_h);
    glTextureId = microTextures[textureId].id;

    microGraphicsDraw(glTextureId,
        v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y,
        tx / tex_w,
        ty / tex_h,
        tw / tex_w,
        th / tex_h,
        r, g, b, a);
  }
  else
  {
    microGraphicsDraw(0,
        v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y,
        0, 0, 0, 0, r, g, b, a);
  }
}

void microGraphicsDrawRect(int textureId, float tx, float ty, float tw, float th,
    float x, float y, float w, float h, float r, float g, float b, float a)
{
  if (textureId == -1) 
  {
    microGraphicsDraw(0, x, y, x + w, y, x + w, y + h, x, y + h, 0, 0, 0, 0, r, g, b, a);
  }
  else
  {
    static int tex_w, tex_h;
    static int glTextureId;
    assert(microTextures[textureId].id != -1);
    microTextureGetSize(textureId, &tex_w, &tex_h);
    glTextureId = microTextures[textureId].id;
    microGraphicsDraw(glTextureId, x, y, x + w, y, x + w, y + h, x, y + h,
        tx / (float)tex_w, ty / (float)tex_h, tw / (float)tex_w,
        th / (float)tex_h, r, g, b, a);
  }
}

//VIEW
void microViewSet(MicroView view)
{
  viewViewportX = view.viewportX;
  viewViewportY = view.viewportY;
  viewViewportW = view.viewportWidth;
  viewViewportH = view.viewportHeight;
  viewCenterX = view.centerX;
  viewCenterY = view.centerY;
  viewWidth = view.width;
  viewHeight = view.height;
  viewRotation = view.rotation;
  viewFlipY = (view.flipY > 0);
  viewUpdated = 0;
}

MicroView microViewGet()
{
  MicroView view;
  view.viewportX = viewViewportX;
  view.viewportY = viewViewportY;
  view.viewportWidth = viewViewportW;
  view.viewportHeight = viewViewportH;
  view.centerX = viewCenterX;
  view.centerY = viewCenterY;
  view.width = viewWidth;
  view.height = viewHeight;
  view.rotation = viewRotation;
  view.flipY = viewFlipY;
  return view;
}

void microViewApply()
{
  if (!viewUpdated) {
    float NCenterX, NCenterY;
    NCenterX = floor(viewCenterX);
    NCenterY = floor(viewCenterY);

    // Rotation components
    float angle = viewRotation * M_PI / 180.f;
    float cosine = cosf(angle);
    float sine = sinf(angle);
    float tx = -NCenterX * cosine - NCenterY * sine + NCenterX;
    float ty = NCenterX * sine - NCenterY * cosine + NCenterY;

    // Projection components
    float a = 2.f / viewWidth;
    float b = viewFlipY ? 2.f / viewHeight : -2.f / viewHeight; // Change the sign of b based on flipY
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

    viewUpdated = 1;
  }

  //Apply view
  if (currentShader != -1)
    microShaderSetMatrix4("u_view", viewMatrix);
}

void microViewFlipY(int flipY)
{
  viewFlipY = (flipY > 0);
  viewUpdated = 0;
}

void microViewSetViewport(float x, float y, float width, float height)
{
  viewViewportX = x;
  viewViewportY = y;
  viewViewportW = width;
  viewViewportH = height;
  viewUpdated = 0;
}

void microViewSetCenter(float x, float y)
{
  viewCenterX = x;
  viewCenterY = y;
  viewUpdated = 0;
}

void microViewSetSize(float width, float height)
{
  viewWidth = width;
  viewHeight = height;
  viewUpdated = 0;
}

void microViewSetRotation(float rotation)
{
  viewRotation = rotation;
  viewUpdated = 0;
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

void microWindowGetSize(int *width, int *height)
{
  SDL_GetWindowSize(window, width, height);
}

void microSwapBuffers()
{
  SDL_GL_SwapWindow(window);
  //SDL_Delay(16);
}
