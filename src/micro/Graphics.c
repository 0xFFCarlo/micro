#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#if defined(__linux__)
// #include <GLES3/gl.h>
// #include <GL/glu.h>
#include <GL/gl.h> #include <GL/glew.h>
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#include <dirent.h>
#include <math.h>

#include "../util/vector.h"
#include "Graphics.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "../util/debug.h"

#define MICRO_MAX_TEXTURES 64
#define MICRO_MAX_CANVASES 64
#define MICRO_MAX_SHADERS 64
#define MICRO_MAX_FONTS 64
#define MICRO_MAX_ANIMATIONS 64
#define MICRO_MAX_ATLASES 16
#define MICRO_MAX_PARTICLE_EMITTERS 64

#define MICRO_MAX_ATTRIBUTES 16
#define MICRO_MAX_UNIFORMS 16
#define MICRO_MAX_NAME_LEN 32
#define MICRO_VERTEX_BUFFER_SIZE (6 * 256)
#define MICRO_MAX_SHADER_LEN 8000
#define MICRO_FONT_TEXTURE_SIZE 1024

#define MICRO_ATLAS_MAX_WIDTH 1024
#define MICRO_ATLAS_MAX_HEIGHT 1024
#define MICRO_ATLAS_MAX_TEXTURES 512

#define MICRO_EMITTER_STEADY 0
#define MICRO_EMITTER_EXPLOSION 1

// GL states
static int currentTexture = 0;
static int currentShader = -1;

typedef struct microTexture
{
  GLuint id;
  int width, height, channels;
} microTexture;
static microTexture microTextures[MICRO_MAX_TEXTURES];

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

typedef struct microAnimation
{
  char name[MICRO_MAX_NAME_LEN];
  int *frames;
  int framesCount;
} microAnimation;
static microAnimation microAnimations[MICRO_MAX_ANIMATIONS];

typedef struct microFont
{
  int textureId;
  int fontSize;
  stbtt_fontinfo fontInfo;
  unsigned char *ttf_buffer;
  int glyphsOffset[128 - 32];
} microFont;
static microFont microFonts[MICRO_MAX_FONTS];
static int microFontsCount = 0;

typedef struct microAtlas
{
  int textureId;
  int width, height;
  char **framesNames;
  MicroTextureSource *frames;
  int framesCount;
} microAtlas;
static microAtlas microAtlases[MICRO_MAX_ATLASES];

typedef struct microParticleEmitter
{
  int x, y;
  uint8_t emitterType;
  int width, height;
  float emissionRate;
  MicroParticle (*particleGenerator)(int);

  float emissionTimer;
  Vector particles;
  Vector freeParticles;
} microParticleEmitter;
static Vector microParticleEmitters;
static Vector microFreedParticleEmitters;
static unsigned int microTotalParticles = 0;

// shader base programs
static const char
  baseVertexShaderSrc[] = "#version 330 core\n"
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

static const char baseFragmentShaderSrc
  [] = "#version 330 core\n"
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

// renderer buffers and states
static SDL_GLContext *context = NULL;
static SDL_Window *window = NULL;
static float vertexbuf[MICRO_VERTEX_BUFFER_SIZE][2];
static float texsourcebuf[MICRO_VERTEX_BUFFER_SIZE][2];
static float colorbuf[MICRO_VERTEX_BUFFER_SIZE][4];
static unsigned int vao;
static unsigned int vbo, cbo, tbo;
static int countverts;
static int wireframe;
static RenderingDebugInfo debugInfo;

// current view state
float viewViewportX, viewViewportY, viewViewportW, viewViewportH;
float viewCenterX, viewCenterY;
float viewWidth, viewHeight;
float viewRotation;
float viewMatrix[16];
int viewFlipY = 0;
int viewUpdated = 0;

////////////////////////////
// GL STATES
////////////////////////////
void microGLStateBindTexture(int textureId)
{
  if (currentTexture == textureId)
    return;
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
// TEXTURE
////////////////////////////
int microBitmapLoadFromFile(const char *filepath, unsigned char **data,
                            unsigned int *width, unsigned int *height,
                            unsigned int *channels)
{
  int w, h, c;
  unsigned char *img = stbi_load(filepath, &w, &h, &c, 0);
  if (img == NULL)
    return -1;
  *data = img;
  *width = w;
  *height = h;
  *channels = c;
  return 0;
}

int microTextureLoadFromFile(const char *filepath)
{
  unsigned char *data;
  unsigned int width, height, channels;
  if (microBitmapLoadFromFile(filepath, &data, &width, &height, &channels) != 0)
    return -1;
  int id = microTextureLoadFromMemory(data, width, height, channels,
                                      MICRO_FILTER_LINEAR);
  stbi_image_free(data);
  return id;
}

int microTextureLoadFromMemory(const unsigned char *data,
                               const unsigned int width,
                               const unsigned int height,
                               const unsigned int channels,
                               const enum MicroFilter filter)
{
  // Get the texture format
  unsigned int fmt = GL_RGBA;
  switch (channels)
  {
  case 1:
    fmt = GL_ALPHA;
    break;
  case 3:
    fmt = GL_RGB;
    break;
  case 4:
    fmt = GL_RGBA;
    break;
  default:
    printf("Error: data file format [%d] not recognized\n", fmt);
    return -1;
  }

  // Create texture
  GLuint id;
  glGenTextures(1, &id);
  assert(id != 0);

  microGLStateBindTexture(id);

  int gl_filter = GL_LINEAR;
  if (filter == MICRO_FILTER_NEAREST)
    gl_filter = GL_NEAREST;
  else if (filter == MICRO_FILTER_LINEAR)
    gl_filter = GL_LINEAR;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  microGLCheckErrors();

  glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE,
               data);

  glFlush();

  // Check for opengl errors
  microGLCheckErrors();

  // find spot in the resources buffer
  microTexture texture;
  texture.id = id;
  texture.width = width;
  texture.height = height;
  texture.channels = channels;
  int spot = -1;
  for (int i = 0; i < MICRO_MAX_TEXTURES; i++)
  {
    if (microTextures[i].id == (GLuint)-1)
    {
      spot = i;
      microTextures[i] = texture;
      break;
    }
  }
  assert(spot != -1);

  return spot;
}

void microTextureSetFilter(const int textureId, const enum MicroFilter filter)
{
  microGLStateBindTexture(microTextures[textureId].id);
  currentTexture = microTextures[textureId].id;

  int gl_filter = GL_LINEAR;
  if (filter == MICRO_FILTER_LINEAR)
    gl_filter = GL_LINEAR;
  else if (filter == MICRO_FILTER_NEAREST)
    gl_filter = GL_NEAREST;
  else
  {
    printf("Error: filter %d not recognized\n", filter);
    return;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
}

void microTextureGetSize(const int textureId, int *width, int *height)
{
  if (width != NULL)
    *width = microTextures[textureId].width;
  if (height != NULL)
    *height = microTextures[textureId].height;
}

void microTextureFree(const int textureId)
{
  if (microTextures[textureId].id != (GLuint)-1)
  {
    glDeleteTextures(1, &microTextures[textureId].id);
    microTextures[textureId].id = -1;
  }
}

/////////////////////////////
/// Texture Atlas
/////////////////////////////
typedef struct
{
  char *name;
  int frame_number;
  int x, y, w, h;
} animation_frame;

// compare animation by name and frame number
int microAnimationFrameCmp(const void *aa, const void *bb)
{
  animation_frame *a = *(animation_frame **)aa;
  animation_frame *b = *(animation_frame **)bb;
  int name_cmp = strcmp(a->name, b->name);
  if (name_cmp != 0)
    return name_cmp;
  return a->frame_number - b->frame_number;
}

void microAtlasGenerateAnimations(int textureAtlasId)
{
  const int verbose = 0;
  microAtlas *atlas = &microAtlases[textureAtlasId];

  animation_frame *animations[MICRO_MAX_ANIMATIONS];
  int animations_count = 0;

  // Parse all animation frames from names
  for (int i = 0; i < atlas->framesCount; i++)
  {
    char *frame_name = atlas->framesNames[i];
    // Skip textures without animation name convention
    if (strchr(frame_name, '_') == NULL)
      continue;

    // Parse animation name and frame number
    uint32_t len = strlen(frame_name);
    char *tmp = malloc(len + 1);
    strcpy(tmp, frame_name);
    tmp[len] = '\0';

    char *animation_name = strtok(tmp, "_");
    char *frame_number_str = strtok(NULL, "_");
    if (animation_name == NULL)
      continue;
    if (frame_number_str == NULL)
      continue;
    assert(strlen(animation_name) > 0);
    assert(strlen(frame_number_str) > 0);

    // parse frame number and make sure it is a number
    int frame_number = atoi(frame_number_str);
    if (frame_number == 0 && frame_number_str[0] != '0')
    {
      free(tmp);
      continue;
    }

    // Get animation source rect
    int x, y, w, h;
    x = atlas->frames[i].x;
    y = atlas->frames[i].y;
    w = atlas->frames[i].w;
    h = atlas->frames[i].h;

    // Store animation frame
    animations[animations_count] = malloc(sizeof(animation_frame));

    size_t animation_name_len = strlen(animation_name); // duplicate string
    animations[animations_count]->name = malloc(animation_name_len + 1);
    strcpy(animations[animations_count]->name, animation_name);
    animations[animations_count]->name[animation_name_len] = '\0';

    animations[animations_count]->frame_number = frame_number;
    animations[animations_count]->x = x;
    animations[animations_count]->y = y;
    animations[animations_count]->w = w;
    animations[animations_count]->h = h;
    animations_count++;
    assert(animations_count < MICRO_MAX_ANIMATIONS);

    free(tmp);
  }

  // Sort animations by names and frame numbers
  qsort(animations, animations_count, sizeof(animation_frame *),
        microAnimationFrameCmp);

  // Create animations from animation frames with same name
  char *last_animation_name = animations[0]->name;
  int frames_params[4 * 1024];
  int current_frame = 0;
  for (int i = 0; i < animations_count; i++)
  {

    if (strcmp(last_animation_name, animations[i]->name) != 0)
    {
      // Create animation
      if (verbose)
        printf("Creating animation %s with %d frames\n", last_animation_name,
               current_frame);
      microAnimationCreateFromFrames(last_animation_name, frames_params,
                                     current_frame);

      // Reset frame params
      current_frame = 0;
    }

    // Store frame params
    frames_params[current_frame * 4 + 0] = animations[i]->x;
    frames_params[current_frame * 4 + 1] = animations[i]->y;
    frames_params[current_frame * 4 + 2] = animations[i]->w;
    frames_params[current_frame * 4 + 3] = animations[i]->h;
    current_frame++;
    assert(current_frame < 1024);

    last_animation_name = animations[i]->name;
  }

  // Create last animation
  if (current_frame)
  {
    if (verbose)
      printf("Creating animation %s with %d frames\n", last_animation_name,
             current_frame);
    microAnimationCreateFromFrames(last_animation_name, frames_params,
                                   current_frame);
  }

  // Free animations frames
  for (int i = 0; i < animations_count; i++)
  {
    free(animations[i]->name);
    free(animations[i]);
  }
}

int microTextureAtlasLoadFromPath(const char *filepath)
{
  const int verbose = 0;

  struct dirent *entry;
  DIR *dir = opendir(filepath);

  if (dir == NULL)
  {
    printf("Error opening directory\n");
    return -1;
  }

  const unsigned int padding = 1;
  stbrp_rect rects[MICRO_ATLAS_MAX_TEXTURES];
  char *filepaths[MICRO_ATLAS_MAX_TEXTURES];
  char filenames[MICRO_ATLAS_MAX_TEXTURES][MICRO_MAX_NAME_LEN];

  // 1. Store all frames and filepaths of images
  int frameCount = 0;
  while ((entry = readdir(dir)) != NULL)
  {

    // Skip `.` and `..` and `.DS_Store`
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
        strcmp(entry->d_name, ".DS_Store") == 0)
      continue;

    // Check if it's a png or jpeg
    if (strstr(entry->d_name, ".png") == NULL &&
        strstr(entry->d_name, ".jpg") == NULL &&
        strstr(entry->d_name, ".jpeg") == NULL)
      continue;

    stbrp_rect *frect = &rects[frameCount];
    char *fname = filenames[frameCount];

    // Store filepath
    filepaths[frameCount] = malloc(strlen(filepath) + strlen(entry->d_name) +
                                   1);
    strcpy(filepaths[frameCount], filepath);
    strcat(filepaths[frameCount], entry->d_name);

    // Store filename without extension
    assert(strlen(entry->d_name) < MICRO_MAX_NAME_LEN);
    strcpy(fname, entry->d_name);
    char *dot = strrchr(fname, '.');
    if (dot)
      *dot = '\0';

    // Load image description and store it
    stbi_info(filepaths[frameCount], &frect->w, &frect->h, NULL);
    frect->id = frameCount;
    frect->x = 0;
    frect->y = 0;
    frect->was_packed = 0;

    // Add padding
    frect->w += padding * 2;
    frect->h += padding * 2;

    if (verbose)
      printf("Loading %s to %s\n", filepaths[frameCount],
             filepaths[frameCount]);

    frameCount++;
    assert(frameCount < MICRO_ATLAS_MAX_TEXTURES);
  }

  // Close the directory
  closedir(dir);

  // 2. Pack frames into atlas
  stbrp_context context;
  stbrp_node nodes[MICRO_ATLAS_MAX_WIDTH * 2];
  stbrp_init_target(&context, MICRO_ATLAS_MAX_WIDTH, MICRO_ATLAS_MAX_HEIGHT,
                    nodes, MICRO_ATLAS_MAX_WIDTH * 2);
  if (verbose)
  {
    printf("Packing %d frames into atlas\n", frameCount);
    printf("Atlas size: %dx%d\n", MICRO_ATLAS_MAX_WIDTH,
           MICRO_ATLAS_MAX_HEIGHT);
  }
  int packedSuccess = stbrp_pack_rects(&context, rects, frameCount);
  if (packedSuccess == 0)
  {
    printf("Could not pack frames into atlas\n");

    for (int i = 0; i < frameCount; i++)
      free(filepaths[i]);

    return -1;
  }

  // 3. Find atlas spot id
  int spot = -1;
  for (int i = 0; i < MICRO_MAX_ATLASES; i++)
  {
    if (microAtlases[i].textureId == -1)
    {
      spot = i;
      break;
    }
  }
  assert(spot != -1);
  microAtlas *atlas = &microAtlases[spot];

  // 4. Allocate the frames array and names
  atlas->frames = malloc(frameCount * sizeof(MicroTextureSource));
  atlas->framesNames = malloc(frameCount * sizeof(char *));
  atlas->framesCount = frameCount;
  atlas->width = MICRO_ATLAS_MAX_WIDTH;
  atlas->height = MICRO_ATLAS_MAX_HEIGHT;
  atlas->textureId = -1;

  // 5. Allocate bitmap for the atlas
  const unsigned int atlasWidth = MICRO_ATLAS_MAX_WIDTH;
  const unsigned int atlasHeight = MICRO_ATLAS_MAX_HEIGHT;
  unsigned char *atlasData = malloc(atlasWidth * atlasHeight * 4);

  // 6. Load each image and store it in the atlas
  for (int frame_id = 0; frame_id < frameCount; frame_id++)
  {

    const char *fullPath = filepaths[frame_id];
    const char *name = filenames[frame_id];
    const stbrp_rect *rect = &rects[frame_id];

    int img_width, img_height, channels;

    if (verbose)
      printf("Loading texture %s\n", fullPath);
    unsigned char *data = stbi_load(fullPath, &img_width, &img_height,
                                    &channels, 0);
    if (data == NULL)
    {
      printf("Error loading texture %s\n", fullPath);
      continue;
    }

    // Check if the texture is too big
    if ((unsigned int)img_width > atlasWidth ||
        (unsigned int)img_height > atlasHeight)
    {
      printf("Error: texture %s is too big\n", fullPath);
      abort();
    }

    // Copy the bitmap to the atlas
    for (int y = 0; y < img_height; y++)
    {
      int tx = rect->x + padding;
      int ty = rect->y + padding + y;
      memcpy(&atlasData[ty * atlasWidth * channels + tx * channels],
             &data[y * img_width * channels], img_width * channels);
    }

    // Free the bitmap
    stbi_image_free(data);

    // Store texture details
    MicroTextureSource source;
    source.w = img_width;
    source.h = img_height;
    source.x = rect->x + padding;
    source.y = rect->y + padding;
    atlas->frames[frame_id] = source;
    if (verbose)
      printf("Stored frame %s (%dx%d) at (%d, %d)\n", name, source.w, source.h,
             source.x, source.y);

    // Store name without file extension
    atlas->framesNames[frame_id] = malloc(strlen(name) + 1);
    strcpy(atlas->framesNames[frame_id], name);
  }

  // 7. Create the texture atlas
  int textureId = microTextureLoadFromMemory(atlasData, atlasWidth, atlasHeight,
                                             4, GL_NEAREST);
  free(atlasData);
  atlas->textureId = textureId;

  // 8. Free the filepaths
  for (int i = 0; i < frameCount; i++)
    free(filepaths[i]);

  // 9. Generate animations from atlas frames
  microAtlasGenerateAnimations(spot);

  return spot;
}

MicroTextureSource microTextureAtlasGetRegion(int textureAtlasId,
                                              const char *name)
{
  // Find the frame id from the name
  int frameId = -1;
  for (int i = 0; i < microAtlases[textureAtlasId].framesCount; i++)
  {
    if (strcmp(microAtlases[textureAtlasId].framesNames[i], name) == 0)
    {
      frameId = i;
      break;
    }
  }
  assert(frameId != -1);
  return microAtlases[textureAtlasId].frames[frameId];
}

int microTextureAtlasGetTextureId(int textureAtlasId)
{
  return microAtlases[textureAtlasId].textureId;
}

void microTextureAtlasFree(int textureAtlasId)
{
  microTextureFree(microAtlases[textureAtlasId].textureId);
  for (int i = 0; i < microAtlases[textureAtlasId].framesCount; i++)
    free(microAtlases[textureAtlasId].framesNames[i]);
  free(microAtlases[textureAtlasId].framesNames);
  free(microAtlases[textureAtlasId].frames);
}

////////////////////////////
// ANIMATION
////////////////////////////
int microAnimationCreateFromFrames(char *name, int *frames, int framesCount)
{
  // find spot in the resources buffer
  int id = -1;
  for (int i = 0; i < MICRO_MAX_ANIMATIONS; i++)
  {
    if (microAnimations[i].framesCount == 0)
    {
      id = i;
      break;
    }
  }
  assert(id != -1);

  // create animation
  assert(strlen(name) < MICRO_MAX_NAME_LEN);
  strcpy(microAnimations[id].name, name);
  microAnimation *animation = &microAnimations[id];
  animation->frames = malloc(sizeof(int) * 4 * framesCount);
  for (int i = 0; i < framesCount; i++)
  {
    animation->frames[i * 4 + 0] = frames[i * 4 + 0];
    animation->frames[i * 4 + 1] = frames[i * 4 + 1];
    animation->frames[i * 4 + 2] = frames[i * 4 + 2];
    animation->frames[i * 4 + 3] = frames[i * 4 + 3];
  }
  microAnimations[id].framesCount = framesCount;

  return id;
}

int microAnimationGet(char *name)
{
  for (int i = 0; i < MICRO_MAX_ANIMATIONS; i++)
  {
    if (strcmp(microAnimations[i].name, name) == 0)
      return i;

    if (microAnimations[i].framesCount == 0)
      break;
  }
  printf("Error: animation %s not found\n", name);
  abort();
  return -1;
}

const char *microAnimationGetName(int animationId)
{
  return microAnimations[animationId].name;
}

MicroTextureSource microAnimationGetFrame(int animationId, int frameId,
                                          int flipX, int flipY)
{
  MicroTextureSource source;
  frameId = frameId % microAnimations[animationId].framesCount;

  int width = microAnimations[animationId].frames[frameId * 4 + 2];
  if (flipX)
    width = -width;
  int height = microAnimations[animationId].frames[frameId * 4 + 3];
  if (flipY)
    height = -height;

  source.x = microAnimations[animationId].frames[frameId * 4 + 0] -
             flipX * width;
  source.y = microAnimations[animationId].frames[frameId * 4 + 1] -
             flipY * height;
  source.w = width;
  source.h = height;
  return source;
}

int microAnimationGetFramesCount(int animationId)
{
  return microAnimations[animationId].framesCount;
}

void microAnimationFree(int animationId)
{
  microAnimations[animationId].framesCount = 0;
  free(microAnimations[animationId].frames);
}

void microAnimationFreeAll()
{
  for (int i = 0; i < MICRO_MAX_ANIMATIONS; i++)
  {
    if (microAnimations[i].framesCount == 0)
      continue;
    microAnimationFree(i);
  }
}

int microFontLoadFromFile(const char *filepath, unsigned int fontSize,
                          int filter)
{
  assert(filter == MICRO_FILTER_NEAREST || filter == MICRO_FILTER_LINEAR);

  FILE *fp = fopen(filepath, "rb");
  if (fp == NULL)
  {
    printf("Failed to open font file\n");
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  unsigned char *ttf_buffer = malloc(size);
  if (ttf_buffer == NULL)
  {
    printf("Failed to allocate memory for ttf_buffer\n");
    fclose(fp);
    return -1;
  }

  fread(ttf_buffer, 1, size, fp);
  fclose(fp);

  stbtt_fontinfo font;
  if (!stbtt_InitFont(&font, ttf_buffer, 0))
  {
    printf("Failed to initialize font\n");
    free(ttf_buffer);
    return -1;
  }

  float scale = stbtt_ScaleForPixelHeight(&font, fontSize);
  assert(isinf(scale) == 0);
  assert(isnan(scale) == 0);

  int x = 0;
  int y = 0;
  int ascent;
  const unsigned int padding = 3;
  stbtt_GetFontVMetrics(&font, &ascent, 0, 0);
  ascent *= scale;
  int glyphOffset[128 - 32];
  unsigned char buffer[MICRO_FONT_TEXTURE_SIZE * MICRO_FONT_TEXTURE_SIZE * 4];
  memset(buffer, 0, sizeof(buffer));

  for (int codepoint = 32; codepoint < 128; codepoint++)
  {
    int advance, lsb, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(&font, codepoint, scale, scale, &x0, &y0, &x1,
                                &y1);

    // render character to buffer
    int width = x1 - x0;
    int height = y1 - y0;
    unsigned char *bitmap = malloc(width * height);
    stbtt_MakeCodepointBitmap(&font, bitmap, width, height, width, scale, scale,
                              codepoint);

    // move to next line if there's no room for the character
    if (x + width >= MICRO_FONT_TEXTURE_SIZE)
    {
      x = 0;
      y += ascent + padding;
    }

    // copy bitmap to buffer
    assert(x + width < MICRO_FONT_TEXTURE_SIZE);
    assert(y + height < MICRO_FONT_TEXTURE_SIZE);
    glyphOffset[codepoint - 32] = x + y * MICRO_FONT_TEXTURE_SIZE;
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
      {
        int px = x + j;
        int py = y + i;
        if (px >= 0 && px < MICRO_FONT_TEXTURE_SIZE && py >= 0 &&
            py < MICRO_FONT_TEXTURE_SIZE)
        {
          buffer[(py * MICRO_FONT_TEXTURE_SIZE + px) * 4 + 0] = 255;
          buffer[(py * MICRO_FONT_TEXTURE_SIZE + px) * 4 + 1] = 255;
          buffer[(py * MICRO_FONT_TEXTURE_SIZE + px) * 4 + 2] = 255;
          buffer[(py * MICRO_FONT_TEXTURE_SIZE + px) * 4 +
                 3] = bitmap[i * width + j];
        }
      }
    }

    free(bitmap);

    // move to next character
    x += advance * scale + padding;
  }

  // find spot
  int id = -1;
  for (int i = 0; i < MICRO_MAX_FONTS; i++)
  {
    if (microFonts[i].textureId == -1)
    {
      id = i;
      break;
    }
  }
  assert(id != -1);
  microFontsCount++;

  // create texture and store data
  microFonts[id].textureId = microTextureLoadFromMemory(buffer,
                                                        MICRO_FONT_TEXTURE_SIZE,
                                                        MICRO_FONT_TEXTURE_SIZE,
                                                        4, filter);
  microFonts[id].fontSize = fontSize;
  microFonts[id].fontInfo = font;
  microFonts[id].ttf_buffer = ttf_buffer;
  memcpy(microFonts[id].glyphsOffset, glyphOffset, sizeof(glyphOffset));

  return id;
}

int microFontGetSize(int fontId)
{
  return microFonts[fontId].fontSize;
}

int microFontGetTextureId(int fontId)
{
  return microFonts[fontId].textureId;
}

int microFontGetLineHeight(int fontId)
{
  int ascent;
  stbtt_GetFontVMetrics(&microFonts[fontId].fontInfo, &ascent, 0, 0);
  return ascent * microFonts[fontId].fontSize;
}

int microFontGetTextWidth(int fontId, const char *text)
{
  const stbtt_fontinfo *fontInfo = &microFonts[fontId].fontInfo;
  const float scale = stbtt_ScaleForPixelHeight(fontInfo,
                                                microFonts[fontId].fontSize);

  float totalWidth = 0.0f;
  float lineWidth = 0.0f;
  char prevChar = '\n';

  for (const char *p = text; *p; ++p)
  {
    char c = *p;

    if (c == '\n')
    {
      // Reset for a new line
      if (lineWidth > totalWidth)
        totalWidth = lineWidth;
      lineWidth = 0.0f;
      prevChar = c;
      continue;
    }

    assert((int)c >= 32 && (int)c <= 126);

    int advance, lsb;
    stbtt_GetCodepointHMetrics(fontInfo, c, &advance, &lsb);

    // Add kerning if it's not the start of a new line
    if (prevChar != '\n')
    {
      float kerning = stbtt_GetCodepointKernAdvance(fontInfo, prevChar, c) *
                      scale;
      lineWidth += kerning;
    }

    // Apply the left side bearing (lsb) and the advance of the character
    lineWidth += lsb * scale;
    lineWidth += (advance - lsb) * scale;

    prevChar = c;
  }

  // In case the string doesn't end with a newline, make sure we still update
  // totalWidth
  if (lineWidth > totalWidth)
    totalWidth = lineWidth;

  return totalWidth;
}

void microFontFree(int fontId)
{
  if (fontId < 0 || fontId >= MICRO_MAX_FONTS)
  {
    printf("Error: invalid font id %d\n", fontId);
    return;
  }

  if (microFonts[fontId].textureId == 0)
  {
    printf("Error: font %d is not loaded\n", fontId);
    return;
  }

  microTextureFree(microFonts[fontId].textureId);
  free(microFonts[fontId].ttf_buffer);
  microFonts[fontId].textureId = 0;
  microFontsCount--;
}

void microFontFreeAll()
{
  for (int i = 0; i < MICRO_MAX_FONTS; i++)
  {
    if (microFonts[i].textureId == -1)
      continue;
    microFontFree(i);
  }
}

////////////////////////////
// SHADER
////////////////////////////
void readShaderFile(const char *filepath, char *dst)
{
  FILE *file = fopen(filepath, "rb");
  if (!file)
  {
    printf("Error: can't open the file %s\n", filepath);
    fclose(file);
    return;
  }

  // Get file size
  fseek(file, 0L, SEEK_END);
  int fsize = (int)ftell(file);
  assert(fsize < MICRO_MAX_SHADER_LEN);

  // Seek to the beginning
  fseek(file, 0L, SEEK_SET);

  // Read all the file into filedata
  fread(dst, sizeof(char) * fsize, 1, file);
  dst[fsize] = '\0';
  fclose(file);
}

int microShaderLoadFromFile(const char *vertexShaderPath,
                            const char *fragmentShaderPath)
{
  char vertShaderSrc[MICRO_MAX_SHADER_LEN];
  char fragShaderSrc[MICRO_MAX_SHADER_LEN];
  readShaderFile(vertexShaderPath, &vertShaderSrc[0]);
  readShaderFile(fragmentShaderPath, &fragShaderSrc[0]);
  return microShaderLoadFromSource(&vertShaderSrc[0], &fragShaderSrc[0]);
}

int microShaderLoadFromSource(const char *vertexShaderSrc,
                              const char *fragmentShaderSrc)
{
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

  GLint result = GL_FALSE;
  int logLength;

  // Compile vertex shader

  const char *vertShaderSrcPtr = &vertexShaderSrc[0];
  glShaderSource(vertShader, 1, &vertShaderSrcPtr, NULL);
  glCompileShader(vertShader);

  char errorLog[4096];

  // Check vertex shader
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
  glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
  assert(logLength < 4096);
  glGetShaderInfoLog(vertShader, logLength, NULL, &errorLog[0]);
  if (logLength != 0)
  {
    printf("Error compiling vertex shader\n");
    printf("%s\n", &errorLog[0]);
  }

  // Compile fragment shader

  const char *fragShaderSrcPtr = &fragmentShaderSrc[0];
  glShaderSource(fragShader, 1, &fragShaderSrcPtr, NULL);
  glCompileShader(fragShader);

  // Check fragment shader

  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
  glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
  assert(logLength < 4096);
  glGetShaderInfoLog(fragShader, logLength, NULL, &errorLog[0]);
  if (logLength != 0)
  {
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
  assert(shader.programId != (GLuint)-1);

  // Load uniforms
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &shader.uniformsCount);
  for (int i = 0; i < shader.uniformsCount; i++)
  {
    GLsizei length;
    GLint size;
    glGetActiveUniform(program, (GLuint)i, MICRO_MAX_NAME_LEN, &length, &size,
                       &shader.uniformsTypes[i], shader.uniformsNames[i]);
    shader.uniformsLocations[i] = glGetUniformLocation(program,
                                                       shader.uniformsNames[i]);
    shader.uniformsValues[i] = 0.0;
  }

  // Find a spot to store the shader
  int spot = -1;
  for (int i = 0; i < MICRO_MAX_SHADERS; i++)
  {
    if (microShaders[i].programId == (GLuint)-1)
    {
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
  if (shaderId == -1 || microShaders[shaderId].programId == (GLuint)-1)
    return;
  if (currentShader == shaderId)
    currentShader = -1;
  glDeleteProgram(microShaders[shaderId].programId);
  microShaders[shaderId].programId = -1;
}

void microShaderApply(int shaderId)
{
  if (currentShader == shaderId)
    return;
  glUseProgram(microShaders[shaderId].programId);
  currentShader = shaderId;
  debugInfo.shaderSwitches++;
}

void microShaderSetUniform(const char *name, ...)
{
  int uniformLoc = -1;
  int uniformId = -1;
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      uniformId = i;
      uniformLoc = shader->uniformsLocations[i];
    }
  }
  assert(uniformId != -1); // Uniform not found

  va_list args;
  va_start(args, name);
  switch (shader->uniformsTypes[uniformId])
  {
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
    assert(0); // Not implemented
    break;
  default:
    assert(0); // Unsupported uniform type
  }
}

void microShaderGetUniform1(const char *name, double *v1)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      *v1 = shader->uniformsValues[i * 4];
      return;
    }
  }
  assert(0); // Uniform not found
}

void microShaderGetUniform2(const char *name, double *v1, double *v2)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      return;
    }
  }
  assert(0); // Uniform not found
}

void microShaderGetUniform3(const char *name, double *v1, double *v2,
                            double *v3)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      *v3 = shader->uniformsValues[i * 4 + 2];
      return;
    }
  }
  assert(0); // Uniform not found
}

void microShaderGetUniform4(const char *name, double *v1, double *v2,
                            double *v3, double *v4)
{
  assert(currentShader != -1);
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      *v1 = shader->uniformsValues[i * 4];
      *v2 = shader->uniformsValues[i * 4 + 1];
      *v3 = shader->uniformsValues[i * 4 + 2];
      *v4 = shader->uniformsValues[i * 4 + 3];
      return;
    }
  }
  assert(0); // Uniform not found
}

void microShaderSetMatrix4(const char *name, float *matrix)
{
  int uniformLoc = -1;
  int uniformId = -1;
  microShader *shader = &microShaders[currentShader];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
    {
      uniformId = i;
      uniformLoc = shader->uniformsLocations[i];
    }
  }
  assert(uniformId != -1); // Uniform not found
  glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, matrix);
}

////////////////////////////
/// CANVAS
///////////////////////////
int microCanvasCreate(int width, int height)
{
  // create framebuffer
  GLuint framebufferId = 0;
  glGenFramebuffers(1, &framebufferId);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
  glViewport(0, 0, width, height);
  assert(framebufferId != 0);

  // create black texture
  const int textureId = microTextureLoadFromMemory(0, width, height, 4,
                                                   MICRO_FILTER_NEAREST);
  assert(textureId != -1);
  assert(microTextures[textureId].id != 0);
  microGLCheckErrors();

  // find spot in the resources buffer
  microCanvas canvas;
  canvas.framebufferId = framebufferId;
  canvas.microTextureId = textureId;
  canvas.width = width;
  canvas.height = height;
  int canvasId = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++)
  {
    if (microCanvases[i].framebufferId == (GLuint)-1)
    {
      canvasId = i;
      microCanvases[i] = canvas;
      break;
    }
  }
  assert(canvasId != -1);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         microTextures[textureId].id, 0);
  microGLCheckErrors();

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
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
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    printf("Failed to initialize th SDL2 library\n");
    return;
  }

  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  const unsigned int screenWidth = DM.w;
  const unsigned int screenHeight = DM.h;

  // create window and opengl context
  window = SDL_CreateWindow("micro", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight,
                            SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL |
                              SDL_WINDOW_SHOWN
                            // SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
  );

  // Set fullscreen
  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

  if (!window)
  {
    printf("Failed to create the window\n");
    return;
  }

  // Hide cursor
  // SDL_ShowCursor(SDL_DISABLE);

  // clear memory allocations
  for (int i = 0; i < MICRO_MAX_TEXTURES; i++)
    microTextures[i].id = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++)
    microCanvases[i].framebufferId = -1;
  for (int i = 0; i < MICRO_MAX_CANVASES; i++)
    microShaders[i].programId = -1;
  for (int i = 0; i < MICRO_MAX_ANIMATIONS; i++)
    microAnimations[i].framesCount = 0;
  for (int i = 0; i < MICRO_MAX_FONTS; i++)
    microFonts[i].textureId = -1;
  for (int i = 0; i < MICRO_MAX_ATLASES; i++)
    microAtlases[i].textureId = -1;
  microParticleEmitters = vector_create(sizeof(microParticleEmitter));
  microFreedParticleEmitters = vector_create(sizeof(int));

  // Set OpenGL version
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // Initilize Opengl
  context = SDL_GL_CreateContext(window);
  if (context == NULL)
  {
    printf("Couldn't create OpenGL context\n");
  }

  // Set VSYNC, try adaptive first and if not supported, use normal vsync
  if (SDL_GL_SetSwapInterval(-1) == -1)
    SDL_GL_SetSwapInterval(1);
  // SDL_GL_SetSwapInterval(0);

  char *glVersion = (char *)glGetString(GL_VERSION);
  if (glVersion)
  {
    debug_print("OpenGL version: %s\n", glVersion);
  }

#if defined(__linux__)
  GLenum err = glewInit();
  if (err != GLEW_OK)
    exit(1);
  else
    printf("GLEW version: %s\n", glewGetString(GLEW_VERSION));
#endif

  // Load base shader
  defaultShaderId = microShaderLoadFromSource(baseVertexShaderSrc,
                                              baseFragmentShaderSrc);
  int programId = microShaderGetProgramID(defaultShaderId);
  assert(defaultShaderId != -1);
  microShaderApply(defaultShaderId);
  const int positionLoc = glGetAttribLocation(programId, "position");
  const int texCoordLoc = glGetAttribLocation(programId, "texcoord");
  const int colorLoc = glGetAttribLocation(programId, "color");

  // Clear
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0, 0.0, 0.0, 0.0);

  // Enable Alpha
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  microGLCheckErrors();

  // create VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  microGLCheckErrors();

  // Create vertex buffer
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 2,
               &vertexbuf[0], GL_STATIC_DRAW);
  glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(positionLoc);
  microGLCheckErrors();

  // Create texture coordinates buffer
  glGenBuffers(1, &tbo);
  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 2,
               &texsourcebuf[0], GL_STATIC_DRAW);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(texCoordLoc);
  microGLCheckErrors();

  // Create color buffer
  glGenBuffers(1, &cbo);
  glBindBuffer(GL_ARRAY_BUFFER, cbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MICRO_VERTEX_BUFFER_SIZE * 4,
               &colorbuf[0], GL_STATIC_DRAW);
  glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);
  microGLCheckErrors();

  // Setup renderer
  countverts = 0;
  wireframe = 0;

  debug_print("microGraphics allocated %d kb\n",
              (int)((sizeof(microTextures) + sizeof(microCanvases) +
                     sizeof(microShaders) + sizeof(microAnimations) +
                     sizeof(microFonts)) /
                    (double)(1024)));

  // Clear debug info
  debugInfo.drawCalls = 0;
  debugInfo.triangles = 0;
  debugInfo.textureSwitches = 0;
  debugInfo.shaderSwitches = 0;
}

void microGraphicsQuit()
{
  // Delete VAO
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &tbo);
  glDeleteBuffers(1, &cbo);

  microAnimationFreeAll();
  microFontFreeAll();
  microParticleEmitterRemoveAll();

  vector_free(&microParticleEmitters);
  vector_free(&microFreedParticleEmitters);

  // Delete context
  SDL_GL_DeleteContext(context);
  // Causes crash sometimes
  SDL_DestroyWindow(window);
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
  glViewport(0, 0, microCanvases[canvasId].width,
             microCanvases[canvasId].height);
  microGLCheckErrors();
}

void microGraphicsDisplay()
{
  if (!countverts)
    return;

  debugInfo.drawCalls++;
  debugInfo.triangles += countverts / 3;

  // Create vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 2, &vertexbuf[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, tbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 2,
               &texsourcebuf[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, cbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * countverts * 4, &colorbuf[0],
               GL_STATIC_DRAW);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, countverts);
  microGLCheckErrors();

  countverts = 0;
}

void microGraphicsDraw(int textureId, float x, float y, float x2, float y2,
                       float x3, float y3, float x4, float y4, float tx_x,
                       float tx_y, float tx_w, float tx_h, float r, float g,
                       float b, float a)
{
  if (textureId == -1)
  {
    if (currentTexture != 0)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(0);
    }
  }
  else
  {
    if (currentTexture != textureId)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(textureId);
      debugInfo.textureSwitches++;
    }
  }

  if (countverts == MICRO_VERTEX_BUFFER_SIZE)
    microGraphicsDisplay();

  unsigned int buffersize = countverts;

  // V1
  vertexbuf[buffersize][0] = x;
  vertexbuf[buffersize][1] = y;

  texsourcebuf[buffersize][0] = tx_x;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  // V2
  vertexbuf[buffersize][0] = x2;
  vertexbuf[buffersize][1] = y2;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  // V3
  vertexbuf[buffersize][0] = x4;
  vertexbuf[buffersize][1] = y4;

  texsourcebuf[buffersize][0] = tx_x;
  texsourcebuf[buffersize][1] = tx_y + tx_h;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  // V4
  vertexbuf[buffersize][0] = x2;
  vertexbuf[buffersize][1] = y2;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  // V5
  vertexbuf[buffersize][0] = x3;
  vertexbuf[buffersize][1] = y3;

  texsourcebuf[buffersize][0] = tx_x + tx_w;
  texsourcebuf[buffersize][1] = tx_y + tx_h;

  colorbuf[buffersize][0] = r;
  colorbuf[buffersize][1] = g;
  colorbuf[buffersize][2] = b;
  colorbuf[buffersize][3] = a;

  buffersize++;

  // V6
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

void microGraphicsDrawRectRot(int textureId, float tx, float ty, float tw,
                              float th, float x, float y, float w, float h,
                              float originX, float originY, float rotation,
                              float r, float g, float b, float a)
{
  static float v1X, v1Y;
  static float v2X, v2Y;
  static float v3X, v3Y;
  static float v4X, v4Y;

  if (rotation)
  {
    const float ca = cosf(rotation);
    const float sa = sinf(rotation);

    const float cx = x + originX;
    const float cy = y + originY;

    const float dx1 = x - cx;
    const float dy1 = y - cy;
    const float dx2 = (x + w) - cx;
    const float dy2 = (y + h) - cy;

    v1X = ca * dx1 - sa * dy1 + cx - originX;
    v1Y = sa * dx1 + ca * dy1 + cy - originY;

    v2X = ca * dx2 - sa * dy1 + cx - originX;
    v2Y = sa * dx2 + ca * dy1 + cy - originY;

    v3X = ca * dx2 - sa * dy2 + cx - originX;
    v3Y = sa * dx2 + ca * dy2 + cy - originY;

    v4X = ca * dx1 - sa * dy2 + cx - originX;
    v4Y = sa * dx1 + ca * dy2 + cy - originY;
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
    assert(microTextures[textureId].id != (GLuint)-1);
    microTextureGetSize(textureId, &tex_w, &tex_h);
    glTextureId = microTextures[textureId].id;

    microGraphicsDraw(glTextureId, v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y,
                      tx / tex_w, ty / tex_h, tw / tex_w, th / tex_h, r, g, b,
                      a);
  }
  else
  {
    microGraphicsDraw(-1, v1X, v1Y, v2X, v2Y, v3X, v3Y, v4X, v4Y, 0, 0, 0, 0, r,
                      g, b, a);
  }
}

void microGraphicsDrawRect(int textureId, float tx, float ty, float tw,
                           float th, float x, float y, float w, float h,
                           float r, float g, float b, float a)
{
  if (textureId == -1)
  {
    microGraphicsDraw(-1, x, y, x + w, y, x + w, y + h, x, y + h, 0, 0, 0, 0, r,
                      g, b, a);
  }
  else
  {
    static int tex_w, tex_h;
    static int glTextureId;
    assert(microTextures[textureId].id != (GLuint)-1);
    microTextureGetSize(textureId, &tex_w, &tex_h);
    glTextureId = microTextures[textureId].id;
    microGraphicsDraw(glTextureId, x, y, x + w, y, x + w, y + h, x, y + h,
                      tx / (float)tex_w, ty / (float)tex_h, tw / (float)tex_w,
                      th / (float)tex_h, r, g, b, a);
  }
}

void microGraphicsDrawText(int fontId, const char *text, float x, float y,
                           float lineSpacing, float r, float g, float b,
                           float a)
{
  const unsigned int textLen = strlen(text);
  const stbtt_fontinfo *fontInfo = &microFonts[fontId].fontInfo;

  const float startX = x;
  const float scale = stbtt_ScaleForPixelHeight(fontInfo,
                                                microFonts[fontId].fontSize);
  assert(isnan(scale) == 0);
  assert(isinf(scale) == 0);
  int ascent;
  int descent;
  stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, 0);
  ascent *= scale;
  assert(isnan(ascent) == 0);

  char prevChar = '\n';
  for (unsigned int i = 0; i < textLen; i++)
  {
    const char c = text[i];
    if (c == '\n')
    {
      y += ascent + lineSpacing;
      x = startX;
      prevChar = c;
      continue;
    }
    assert((int)c >= 32 && (int)c <= 126);

    // Get glyph metrics and position in texture atlas
    const int glyphIndex = microFonts[fontId].glyphsOffset[(int)c - 32];
    int advance, lsb, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(fontInfo, c, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(fontInfo, c, scale, scale, &x0, &y0, &x1, &y1);

    // Compute glyph texture coordinates
    const float tx = glyphIndex % MICRO_FONT_TEXTURE_SIZE;
    const float ty = (int)(glyphIndex / MICRO_FONT_TEXTURE_SIZE);
    const float tw = x1 - x0;
    const float th = y1 - y0;

    // Added kerning adjustment
    if (prevChar != '\n')
    {
      float kerning = stbtt_GetCodepointKernAdvance(fontInfo, prevChar, c) *
                      scale;
      assert(isnan(kerning) == 0);
      x += kerning;
    }

    // Apply the left side bearing to the next character
    if (prevChar != '\n')
      x += lsb * scale;

    // Draw glyph
    microGraphicsDrawRect(microFontGetTextureId(fontId), tx, ty, tw, th, x,
                          y + y0, tw, th, r, g, b, a);

    x += (advance - lsb) * scale;
    prevChar = c;
  }
}

// VIEW
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
  if (!viewUpdated)
  {
    float NCenterX, NCenterY;
    NCenterX = floor(viewCenterX);
    NCenterY = floor(viewCenterY);

    // Rotation components
    float angle = viewRotation;
    float cosine = cosf(angle);
    float sine = sinf(angle);
    float tx = -NCenterX * cosine - NCenterY * sine + NCenterX;
    float ty = NCenterX * sine - NCenterY * cosine + NCenterY;

    // Projection components
    float a = 2.f / viewWidth;
    float b = viewFlipY
                ? 2.f / viewHeight
                : -2.f / viewHeight; // Change the sign of b based on flipY
    float c = -a * NCenterX;
    float d = -b * NCenterY;

    const float zNear = 0;
    const float zFar = 10;
    const float e = 2.f / (zFar - zNear);
    const float f = -(zFar + zNear) / (zFar - zNear);

    // Update matrix
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

    // Set viewport
    glViewport(viewViewportX, viewViewportY, viewViewportW, viewViewportH);

    viewUpdated = 1;
  }

  // Apply view
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

void microViewGetCenter(float *centerX, float *centerY)
{
  *centerX = viewCenterX;
  *centerY = viewCenterY;
}

void microViewGetSize(float *width, float *height)
{
  *width = viewWidth;
  *height = viewHeight;
}

float microViewGetRotation()
{
  return viewRotation;
}

void microViewGetViewport(float *width, float *height)
{
  *width = viewViewportW;
  *height = viewViewportH;
}

void microViewPointWorldToScreen(float x, float y, float *outX, float *outY)
{
  // 1. Translate to the view's local coordinate system (center becomes origin)
  x -= viewCenterX;
  y -= viewCenterY;

  // 2. Rotate around the (new) origin
  const float angle = -viewRotation;
  const float cosine = cosf(angle);
  const float sine = sinf(angle);
  float tx = x * cosine - y * sine;
  float ty = x * sine + y * cosine;

  // 3. Scale (if required)
  const float scaleX = viewViewportW / viewWidth;
  const float scaleY = viewViewportH / viewHeight;
  tx *= scaleX;
  ty *= scaleY;

  // 4. Translate to screen coordinates (from the origin to the center of the
  // viewport)
  *outX = tx + viewViewportW / 2;
  *outY = ty + viewViewportH / 2;
}

void microViewPointScreenToWorld(float x, float y, float *outX, float *outY)
{
  // 1. Inverse Translation to the origin
  x -= viewViewportW / 2;
  y -= viewViewportH / 2;

  // 2. Inverse Scaling
  const float invScaleX = viewWidth / viewViewportW;
  const float invScaleY = viewHeight / viewViewportH;
  x *= invScaleX;
  y *= invScaleY;

  // 3. Inverse Rotation
  const float angle = viewRotation;
  const float cosine = cosf(angle);
  const float sine = sinf(angle);
  float tx = x * cosine +
             y * sine; // Note: inverse rotation uses + for sine in x
  float ty = -x * sine +
             y * cosine; // Note: inverse rotation uses - for sine in y

  // 4. Inverse centering translation
  *outX = tx + viewCenterX;
  *outY = ty + viewCenterY;
}

RenderingDebugInfo microGetRenderingDebugInfo()
{
  return debugInfo;
}

void microRenderingDebugInfoClear()
{
  // Clear debug info
  debugInfo.drawCalls = 0;
  debugInfo.triangles = 0;
  debugInfo.textureSwitches = 0;
  debugInfo.shaderSwitches = 0;
}

void microWindowGetSize(int *width, int *height)
{
  SDL_GetWindowSize(window, width, height);
}

void microSwapBuffers()
{
  SDL_GL_SwapWindow(window);
}

Uint32 lastTime = 0;
float microGraphicsDelayToNextFrame(float target_fps)
{
  // If frame finished early
  int frame_time = SDL_GetTicks() - lastTime;
  if (1000 / target_fps > frame_time)
  {
    // Wait the remaining time
    SDL_Delay(1000 / target_fps - frame_time);
  }

  // Calculate delta time
  frame_time = SDL_GetTicks() - lastTime;
  float deltaTime = (float)frame_time / 1000.0;
  lastTime = SDL_GetTicks();

  return deltaTime;
}

////////////////////////////
/// Particle emitter
////////////////////////////
int microParticleEmitterCreateSteady(int x, int y, float emissionRate,
                                     MicroParticle (*generationFunc)(int))
{
  // Create emitter
  microParticleEmitter newEmitter;
  newEmitter.particles = vector_create(sizeof(MicroParticle));
  newEmitter.freeParticles = vector_create(sizeof(int));
  newEmitter.particleGenerator = generationFunc;
  newEmitter.emitterType = MICRO_EMITTER_STEADY;
  newEmitter.emissionTimer = 0;
  newEmitter.emissionRate = emissionRate;
  newEmitter.width = 1;
  newEmitter.height = 1;
  newEmitter.x = x;
  newEmitter.y = y;

  int spot = -1;
  if (microFreedParticleEmitters.size > 0)
  {
    // Reuse a freed emitter
    spot = *(int *)vector_back(&microFreedParticleEmitters);
    vector_pop_back(&microFreedParticleEmitters);
    memcpy(vector_at(&microParticleEmitters, spot), &newEmitter,
           sizeof(microParticleEmitter));
  }
  else
  {
    vector_push_back(&microParticleEmitters, &newEmitter);
    spot = microParticleEmitters.size - 1;
  }

  return spot;
}

int microParticleEmitterCreateExplosion(int x, int y, int particlesCount,
                                        MicroParticle (*generationFunc)(int))
{
  // Create emitter
  microParticleEmitter newEmitter;
  newEmitter.particles = vector_create(sizeof(MicroParticle));
  newEmitter.freeParticles = vector_create(sizeof(int));
  newEmitter.particleGenerator = generationFunc;
  newEmitter.emitterType = MICRO_EMITTER_EXPLOSION;
  newEmitter.emissionTimer = 0;
  newEmitter.emissionRate = 0;
  newEmitter.width = 1;
  newEmitter.height = 1;
  newEmitter.x = x;
  newEmitter.y = y;

  int spot = -1;
  if (microFreedParticleEmitters.size > 0)
  {
    // Reuse a freed emitter
    spot = *(int *)vector_back(&microFreedParticleEmitters);
    vector_pop_back(&microFreedParticleEmitters);
    memcpy(vector_at(&microParticleEmitters, spot), &newEmitter,
           sizeof(microParticleEmitter));
  }
  else
  {
    vector_push_back(&microParticleEmitters, &newEmitter);
    spot = microParticleEmitters.size - 1;
  }
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, spot);

  // Create particles if it is an explosion
  for (int i = 0; i < particlesCount; i++)
  {
    MicroParticle p = generationFunc(spot);
    p.x = emitter->x - (float)emitter->width / 2 + rand() % emitter->width;
    p.y = emitter->y - (float)emitter->height / 2 + rand() % emitter->height;
    p.life = p.maxLife;
    p.alpha = p.startAlpha;
    p.scale = p.startScale;
    p.alive = 1;
    vector_push_back(&emitter->particles, &p);
    microTotalParticles++;
  }

  return spot;
}

void microParticleEmitterSetPosition(int emitterId, int x, int y)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  emitter->x = x;
  emitter->y = y;
}

void microParticleEmitterSetSize(int emitterId, int width, int height)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  emitter->width = width;
  emitter->height = height;
}

void microParticleEmitterSetEmissionRate(int emitterId, float emissionRate)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  emitter->emissionRate = emissionRate;
}

void microParticleEmitterSetGenerationFunc(int emitterId,
                                           MicroParticle (*generationFunc)(int))
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  emitter->particleGenerator = generationFunc;
}
void microParticleEmitterGetPosition(int emitterId, int *x, int *y)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  *x = emitter->x;
  *y = emitter->y;
}

float microParticleEmitterGetEmissionRate(int emitterId)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  return emitter->emissionRate;
}

void microParticleEmittersUpdate(float dt)
{
  for (size_t i = 0; i < microParticleEmitters.size; i++)
  {

    microParticleEmitter *emitter = vector_at(&microParticleEmitters, i);
    Vector *particles = &emitter->particles;

    // Update particles
    for (unsigned int j = 0; j < particles->size; j++)
    {
      MicroParticle *p = vector_at(particles, j);
      if (p->life <= 0 || !p->alive)
        continue;
      p->x += p->vx * dt;
      p->y += p->vy * dt;
      p->life -= dt;
      p->alpha = p->startAlpha * (p->life / p->maxLife) +
                 p->endAlpha * (1 - p->life / p->maxLife);
      p->rotation += p->rotationSpeed * dt;
      p->scale = p->startScale * (p->life / p->maxLife) +
                 p->endScale * (1 - p->life / p->maxLife);
    }

    // Remove dead particles
    for (unsigned int j = 0; j < particles->size; j++)
    {
      MicroParticle *p = vector_at(particles, j);
      if (p->life <= 0 && p->alive)
      {
        p->alive = 0;
        vector_push_back(&emitter->freeParticles, &j);
        microTotalParticles--;
      }
    }

    // Emit new particles
    if (emitter->emitterType == MICRO_EMITTER_STEADY)
    {
      emitter->emissionTimer += dt;

      if (emitter->emissionTimer > 1.0 / emitter->emissionRate)
      {
        MicroParticle p = emitter->particleGenerator(i);
        p.x = emitter->x - (float)emitter->width / 2 + rand() % emitter->width;
        p.y = emitter->y - (float)emitter->height / 2 +
              rand() % emitter->height;
        p.life = p.maxLife;
        p.alpha = p.startAlpha;
        p.scale = p.startScale;
        p.alive = 1;
        microTotalParticles++;

        if (emitter->freeParticles.size > 0)
        {
          int spot = *(int *)vector_back(&emitter->freeParticles);
          vector_pop_back(&emitter->freeParticles);
          memcpy(vector_at(particles, spot), &p, sizeof(MicroParticle));
        }
        else
        {
          vector_push_back(particles, &p);
        }

        emitter->emissionTimer = 0;
      }
    }
  }
}

void microParticleEmitterDraw(int emitterId)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  Vector *particles = &emitter->particles;

  for (unsigned int i = 0; i < particles->size; i++)
  {
    MicroParticle *p = vector_at(particles, i);
    if (p->life <= 0.0)
      continue;

    microGraphicsDrawRectRot(p->textureId, p->tx, p->ty, p->tw, p->th, p->x,
                             p->y, p->scale, p->scale, p->scale / 2,
                             p->scale / 2, p->rotation, 1.0, 1.0, 1.0,
                             p->alpha);
  }
}

void microParticleEmitterRemove(int emitterId)
{
  microParticleEmitter *emitter = vector_at(&microParticleEmitters, emitterId);
  vector_free(&emitter->particles);
  vector_free(&emitter->freeParticles);
  vector_push_back(&microFreedParticleEmitters, &emitterId);
}

void microParticleEmitterRemoveAll()
{
  for (unsigned int i = 0; i < microParticleEmitters.size; i++)
    microParticleEmitterRemove(i);
}

int microParticlesCount()
{
  return microTotalParticles;
}
