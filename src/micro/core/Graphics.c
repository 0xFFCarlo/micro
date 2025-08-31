#if defined(__linux__)
// #include <GLES3/gl.h>
// #include <GL/glu.h>
// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>
// clang-format on
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif
#include <dirent.h>
#include <math.h>

#include "../util/vector.h"
#include "../util/vmath.h"
#include "Graphics.h"
#include "System.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "../util/Types.h"
#include "../util/debug.h"

#define MICRO_MAX_TEXTURES 64
#define MICRO_MAX_CANVASES 64
#define MICRO_MAX_SHADERS 64
#define MICRO_MAX_FONTS 64
#define MICRO_MAX_ANIMATIONS 64
#define MICRO_MAX_ATLASES 16
#define MICRO_MAX_PARTICLE_EMITTERS 64
#define MICRO_MAX_LIGHTS 256
#define MICRO_MAX_RESOURCES 128

#define MICRO_MAX_ATTRIBUTES 16
#define MICRO_MAX_UNIFORMS 16
#define MICRO_MAX_NAME_LEN 32
#define MICRO_SPRITES_BUFFER_SIZE 1000
#define MICRO_MAX_SHADER_LEN 8000
#define MICRO_FONT_TEXTURE_SIZE 1024

#define MICRO_ATLAS_MAX_WIDTH 2048
#define MICRO_ATLAS_MAX_HEIGHT 2048
#define MICRO_ATLAS_MAX_TEXTURES 512

#define MICRO_EMITTER_STEADY 0
#define MICRO_EMITTER_EXPLOSION 1

typedef enum MicroResourceType
{
  RESOURCE_TEXTURE = 0,
  RESOURCE_TEXTURE_REGION,
  RESOURCE_SHADER,
  RESOURCE_FONT,
  RESOURCE_ANIMATION,
  RESOURCE_ATLAS,
  RESOURCE_PARTICLE_EMITTER,
  RESOURCE_CANVAS,
  RESOURCE_VAO,
  RESOURCE_COUNT,
} MicroResourceType;

// Resource map
typedef struct MicroResource
{
  uint64_t hash;
  int id;
} MicroResource;
static MicroResource resources_map[RESOURCE_COUNT][MICRO_MAX_RESOURCES];
static int resources_inv_map[RESOURCE_COUNT][MICRO_MAX_RESOURCES];

// GL states
static int currentTexture = 0;
static int currentShader = -1;

typedef struct microTexture
{
  GLuint id;
  int width, height, channels;
  GLenum gl_format;
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
  char attributesNames[MICRO_MAX_ATTRIBUTES][MICRO_MAX_NAME_LEN];
  int attributesLocations[MICRO_MAX_ATTRIBUTES];
  GLenum attributesTypes[MICRO_MAX_ATTRIBUTES];
  int attributesCount;
} microShader;
static microShader microShaders[MICRO_MAX_SHADERS];

typedef struct microCanvas
{
  GLuint framebufferId;
  int microTextureId;
  int width, height;
} microCanvas;
static microCanvas microCanvases[MICRO_MAX_CANVASES];

typedef struct MicroLight
{
  bool is_active;
  int cx, cy;
  float radius;
  float intensity;
  float color[3];
} MicroLight;

static int lightsCanvasId = -1;
static MicroLight microLights[MICRO_MAX_LIGHTS];
static uint16_t microLightsCount = 0;
static uint16_t microLightsDeleted[MICRO_MAX_LIGHTS];
static uint16_t microLightsDeletedCount = 0;
static float ambientLightIntensity = 0.0;

typedef struct microAnimation
{
  char name[MICRO_MAX_NAME_LEN];
  int *frames;
  int framesCount;
} microAnimation;
static microAnimation microAnimations[MICRO_MAX_ANIMATIONS];

typedef enum MicroFontType
{
  MICRO_FONT_TTF = 0,
  MICRO_FONT_BITMAP = 1
} MicroFontType;

typedef struct microFontTTF
{
  stbtt_fontinfo fontInfo;
  unsigned char *ttf_buffer;
  int glyphsOffset[128 - 32];
} MicroFontTTF;

typedef struct MicroFontBitmap
{
  MicroTextureRegion tex_source;
  int char_width;
  int char_code_start;
  int char_code_end;
} MicroFontBitmap;

typedef struct MicroFont
{
  MicroFontType type;
  int textureId;
  int fontSize;
  union {
    MicroFontTTF ttf;
    MicroFontBitmap bitmap;
  };
} MicroFont;
static MicroFont microFonts[MICRO_MAX_FONTS];
static int microFontsCount = 0;

typedef struct microAtlas
{
  int textureId;
  int width, height;
  char **framesNames;
  MicroTextureRegion *frames;
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
  MicroParticle *particles;
  int *freeParticles;
} microParticleEmitter;
static microParticleEmitter *microParticleEmitters;
static int *microFreedParticleEmitters;
static unsigned int microTotalParticles = 0;

typedef struct
{
  uint32_t count;
  uint32_t instanceCount;
  uint32_t start;
  uint32_t baseInstance;
} DrawCommand;

typedef struct MicroVAO
{
  int vao_id;
  MicroAttributeData attrs[MICRO_MAX_ATTRIBUTES];
  int attr_count;
  int elements_bo;
  int shaderId;
  int textureGLId;
  uint32_t draw_indirect_buf_id;
  DrawCommand drawCmd;
} MicroVAO;
static MicroVAO *microVAOs;

static const int vao_draw_modes[] = {GL_STATIC_DRAW, GL_DYNAMIC_DRAW,
                                     GL_STREAM_DRAW};

static const int vao_component_sizes[] = {
  sizeof(unsigned char),  // MICRO_BYTE
  sizeof(unsigned char),  // MICRO_UNSIGNED_BYTE
  sizeof(short),          // MICRO_SHORT
  sizeof(unsigned short), // MICRO_UNSIGNED_SHORT
  sizeof(int),            // MICRO_INT
  sizeof(unsigned int),   // MICRO_UNSIGNED_INT
  sizeof(float),          // MICRO_FLOAT
  sizeof(double),         // MICRO_DOUBLE
};

static const GLenum vao_component_types[] = {
  GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT,
  GL_INT,  GL_UNSIGNED_INT,  GL_FLOAT, GL_DOUBLE,
};

// shader base programs
static const char baseVertexShaderSrc
  [] = "#version 330 core\n"
       "layout(location = 0) in vec2 vpos;\n"
       "layout(location = 1) in vec2 position;\n"
       "layout(location = 2) in vec2 size;\n"
       "layout(location = 3) in vec2 origin;\n"
       "layout(location = 4) in float rotation;\n"
       "layout(location = 5) in vec4 texcoord;\n"
       "layout(location = 6) in vec4 color;\n"
       "out vec2 o_uv;\n"
       "flat out vec4 o_color;\n"
       "flat out vec4 o_texcoord;\n"
       "uniform mat4 u_view;\n"
       "void main()\n"
       "{\n"
       "  vec2 vertPos = vpos * size - origin;\n"
       "  vec2 rotatedVertPos = vec2(vertPos.x * cos(rotation) - vertPos.y * "
       "sin(rotation),\n"
       "                             vertPos.x * sin(rotation) + vertPos.y * "
       "cos(rotation));\n"
       "  rotatedVertPos = rotatedVertPos;\n"
       "  vec2 worldPos = position + rotatedVertPos;\n"
       "  worldPos = floor(worldPos);\n"
       "  gl_Position = u_view * vec4(worldPos, 0.0, 1.0);\n"
       "  o_uv = vpos;\n"
       "  o_color = color;\n"
       "  o_texcoord = texcoord;\n"
       "}\n";

static const char
  baseFragmentShaderSrc[] = "#version 330 core\n"
                            "in vec2 o_uv;\n"
                            "flat in vec4 o_color;\n"
                            "flat in vec4 o_texcoord;\n"
                            "out vec4 fragColor;\n"
                            "uniform sampler2D u_texture;\n"
                            "void main()\n"
                            "{\n"
                            "  vec4 texColor = texture(u_texture, "
                            "o_texcoord.xy + o_uv * o_texcoord.zw);\n"
                            "  fragColor = texColor * o_color;\n"
                            "}\n";

static const char lightFragmentShaderSrc
  [] = "#version 330 core\n"
       "in vec2 o_uv;\n"
       "flat in vec4 o_color;\n"
       "flat in vec4 o_texcoord;\n"
       "out vec4 fragColor;\n"
       "void main()\n"
       "{\n"
       "vec4 _unused = o_texcoord * 0.0;\n"
       "vec2 d = o_uv - vec2(0.5f);\n"
       "float distance = length(d) * 2.0;\n"
       "float lightLevel = 1.0 - smoothstep(1.0, 0.0, distance);\n"
       "fragColor = vec4(0.0, 0.0, 0.0, 1.0 * lightLevel * o_color.a);\n"
       "}\n";

static int defaultShaderId = -1;
static int lightShaderId = -1;

// renderer buffers and states
static const float sprites_vertex_buf[6 * 2] = {0, 0, 1, 0, 0, 1,
                                                1, 0, 1, 1, 0, 1};
static float sprites_pos_buf[MICRO_SPRITES_BUFFER_SIZE * 2];
static float sprites_size_buf[MICRO_SPRITES_BUFFER_SIZE * 2];
static float sprites_origin_buf[MICRO_SPRITES_BUFFER_SIZE * 2];
static float sprites_rotation_buf[MICRO_SPRITES_BUFFER_SIZE];
static float sprites_texsrc_buf[MICRO_SPRITES_BUFFER_SIZE * 4];
static unsigned char sprites_color_buf[MICRO_SPRITES_BUFFER_SIZE * 4];
static int defaultVAOId = -1;
static RenderingDebugInfo debugInfo;

// current view state
static MicroView view;
static MicroView3d view3d;

////////////////////////////
// Resources management
////////////////////////////
static uint64_t fnv1a_hash(const char *str)
{
  const uint64_t fnv_prime = 0x100000001b3;
  uint64_t hash = 0xcbf29ce484222325; // FNV offset basis

  while (*str)
  {
    hash ^= (uint64_t)(*str++);
    hash *= fnv_prime;
  }

  return hash;
}

static int microResourceStore(const char *name, MicroResourceType type, int id)
{
  uint64_t hash = fnv1a_hash(name) % MICRO_MAX_RESOURCES;
  int spot = -1;

  for (int i = 0; i < MICRO_MAX_RESOURCES; i++)
  {
    int idx = (hash + i) % MICRO_MAX_RESOURCES;
    if (resources_map[type][idx].hash != 0 || resources_map[type][idx].id != -1)
      continue;
    spot = idx;
    break;
  }

  if (spot == -1)
  {
    printf("Error: no spot found for resource %s\n", name);
    return -1;
  }

  resources_map[type][spot].hash = hash;
  resources_map[type][spot].id = id;
  resources_inv_map[type][id] = spot;

  return spot;
}

static int microResourceRemove(MicroResourceType type, int id)
{
  int spot = resources_inv_map[type][id];
  if (spot == -1)
    return -1;
  resources_map[type][spot].hash = 0;
  resources_map[type][spot].id = -1;
  resources_inv_map[type][id] = -1;
  return 0;
}

static int microResourceGet(MicroResourceType type, const char *name)
{
  uint64_t hash = fnv1a_hash(name) % MICRO_MAX_RESOURCES;
  for (int i = 0; i < MICRO_MAX_RESOURCES; i++)
  {
    int idx = (hash + i) % MICRO_MAX_RESOURCES;
    if (resources_map[type][idx].hash == hash)
      return resources_map[type][idx].id;
  }

  return -1;
}

////////////////////////////
// GL STATES
////////////////////////////
void microGLStateBindTexture(int textureId, int textureUnitId)
{
  if (currentTexture == textureId)
    return;
  glActiveTexture(GL_TEXTURE0 + textureUnitId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  currentTexture = textureId;
  debugInfo.textureSwitches++;
}

#define GL_CHECK_ERRORS()                                                      \
  {                                                                            \
    GLenum err = glGetError();                                                 \
    if (err != GL_NO_ERROR)                                                    \
    {                                                                          \
      printf("OpenGL Error: %d at %s:%d\n", err, __FILE__, __LINE__);          \
      abort_trace();                                                           \
    }                                                                          \
  }

#define GL_CHECK_ERRORS_PRINT(details_format, ...)                             \
  {                                                                            \
    GLenum err = glGetError();                                                 \
    if (err != GL_NO_ERROR)                                                    \
    {                                                                          \
      printf("OpenGL Error: %d at %s:%d\n", err, __FILE__, __LINE__);          \
      printf(details_format, __VA_ARGS__);                                     \
      abort_trace();                                                           \
    }                                                                          \
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

int microTextureLoadFromFile(const char *resName, const char *filepath)
{
  unsigned char *data;
  unsigned int width, height, channels;
  if (microBitmapLoadFromFile(filepath, &data, &width, &height, &channels) != 0)
    return -1;
  int id = microTextureLoadFromMemory(resName, data, width, height, channels,
                                      MICRO_FILTER_LINEAR, 0);
  stbi_image_free(data);
  return id;
}

int microTextureLoadFromMemory(const char *resName, const unsigned char *data,
                               const unsigned int width,
                               const unsigned int height,
                               const unsigned int channels,
                               const enum MicroFilter filter,
                               const unsigned int textureUnitId)
{
  // Get the texture format
  unsigned int fmt = GL_RGBA;
  switch (channels)
  {
  case 1:
    fmt = GL_RED;
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

  microGLStateBindTexture(id, textureUnitId);

  int gl_filter = GL_LINEAR;
  if (filter == MICRO_FILTER_NEAREST)
    gl_filter = GL_NEAREST;
  else if (filter == MICRO_FILTER_LINEAR)
    gl_filter = GL_LINEAR;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GL_CHECK_ERRORS();

  // data may be null
  glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE,
               data);
  glFlush();

  // Check for opengl errors
  GL_CHECK_ERRORS();

  // find spot in the resources buffer
  microTexture texture;
  texture.id = id;
  texture.width = width;
  texture.height = height;
  texture.channels = channels;
  texture.gl_format = fmt;
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

  if (resName != NULL)
    microResourceStore(resName, RESOURCE_TEXTURE, spot);

  debug_print("Texture %d loaded from %s (%dx%d)\n", spot, resName, width,
              height);

  return spot;
}

int microTextureSubmitData(const int textureId, const unsigned int startX,
                           const unsigned int startY, const unsigned int width,
                           const unsigned int height, const unsigned char *data)
{
  glTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, width, height,
                  microTextures[textureId].gl_format, GL_UNSIGNED_BYTE, data);
  return 0;
}

void microTextureSetFilter(const int textureId, const enum MicroFilter filter)
{
  microGLStateBindTexture(microTextures[textureId].id, 0);
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

void microTextureApply(const int textureId, const int textureUnitId)
{
  microGLStateBindTexture(microTextures[textureId].id, textureUnitId);
}

int microTextureGet(const char *resName)
{
  return microResourceGet(RESOURCE_TEXTURE, resName);
}

void microTextureFree(const int textureId)
{
  if (microTextures[textureId].id != (GLuint)-1)
  {
    glDeleteTextures(1, &microTextures[textureId].id);
    microTextures[textureId].id = -1;
    microResourceRemove(RESOURCE_TEXTURE, textureId);
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

// sort strings with numbers in them
// int human_str_sort(const char *a, const char *b) {
//   while (*a && *b) {
//     if (isdigit(*a) && isdigit(*b)) {
//       long int num_a = strtol(a, (char **)&a, 10);
//       long int num_b = strtol(b, (char **)&b, 10);
//       if (num_a < num_b)
//         return -1;
//       if (num_a > num_b)
//         return 1;
//     } else {
//       if (*a < *b)
//         return -1;
//       if (*a > *b)
//         return 1;
//       a++;
//       b++;
//     }
//   }
//
//   if (*a)
//     return 1; // a is longer than b
//   if (*b)
//     return -1; // b is longer than a
//   return 0;    // a and b are the same length
// }

void microAtlasGenerateAnimations(int textureAtlasId)
{
  const int verbose = 0;
  microAtlas *atlas = &microAtlases[textureAtlasId];

  animation_frame *animations[atlas->framesCount];
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

    assert(animations_count < atlas->framesCount);

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

    free(tmp);
  }

  if (animations_count == 0)
    return;

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

int microAtlasLoadFromPath(const char *resName, const char *filepath)
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
  unsigned int spaceUsed = 0;
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

    spaceUsed += frect->w * frect->h;

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
  atlas->frames = malloc(frameCount * sizeof(MicroTextureRegion));
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
    MicroTextureRegion source;
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
  int textureId = microTextureLoadFromMemory(NULL, atlasData, atlasWidth,
                                             atlasHeight, 4, GL_NEAREST, 0);
  free(atlasData);
  atlas->textureId = textureId;

  // 8. Free the filepaths
  for (int i = 0; i < frameCount; i++)
    free(filepaths[i]);

  // 9. Generate animations from atlas frames
  microAtlasGenerateAnimations(spot);

  // 10. Print debug info
  float spaceUsedPercent = (float)spaceUsed / (float)(MICRO_ATLAS_MAX_WIDTH *
                                                      MICRO_ATLAS_MAX_HEIGHT);
  debug_print("Atlas %d loaded! Space used: %.2f%%\n", spot,
              spaceUsedPercent * 100.0f);

  if (resName != NULL)
    microResourceStore(resName, RESOURCE_ATLAS, spot);

  return spot;
}

MicroTextureRegion microAtlasGetRegion(int textureAtlasId, const char *name)
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
  if (frameId == -1)
  {
    printf("Error: frame %s not found in atlas %d\n", name, textureAtlasId);
    abort();
  }
  return microAtlases[textureAtlasId].frames[frameId];
}

int microAtlasGetTextureId(int textureAtlasId)
{
  return microAtlases[textureAtlasId].textureId;
}

int microAtlasGet(const char *resName)
{
  return microResourceGet(RESOURCE_ATLAS, resName);
}

void microAtlasFree(int textureAtlasId)
{
  microTextureFree(microAtlases[textureAtlasId].textureId);
  for (int i = 0; i < microAtlases[textureAtlasId].framesCount; i++)
    free(microAtlases[textureAtlasId].framesNames[i]);
  free(microAtlases[textureAtlasId].framesNames);
  free(microAtlases[textureAtlasId].frames);
  microResourceRemove(RESOURCE_ATLAS, textureAtlasId);
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

MicroTextureRegion microAnimationGetFrame(int animationId, int frameId,
                                          int flipX, int flipY)
{
  MicroTextureRegion source;
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

int microFontTTFLoad(const char *resName, const char *filepath,
                     unsigned int fontSize, int filter)
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
  microFonts[id].type = MICRO_FONT_TTF;
  microFonts[id].textureId = microTextureLoadFromMemory(NULL, buffer,
                                                        MICRO_FONT_TEXTURE_SIZE,
                                                        MICRO_FONT_TEXTURE_SIZE,
                                                        4, filter, 0);
  microFonts[id].fontSize = fontSize;
  microFonts[id].ttf.fontInfo = font;
  microFonts[id].ttf.ttf_buffer = ttf_buffer;
  memcpy(microFonts[id].ttf.glyphsOffset, glyphOffset, sizeof(glyphOffset));

  if (resName != NULL)
    microResourceStore(resName, RESOURCE_FONT, id);

  return id;
}

int microFontBitmapMakeFromPatch(const char *resName, int textureId,
                                 MicroTextureRegion texSource, int charWidth,
                                 int charHeight, int charCodeStart,
                                 int charCodeEnd)
{
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

  microFonts[id].type = MICRO_FONT_BITMAP;
  microFonts[id].textureId = textureId;
  microFonts[id].fontSize = charHeight;
  microFonts[id].bitmap.char_width = charWidth;
  microFonts[id].bitmap.char_code_start = charCodeStart;
  microFonts[id].bitmap.char_code_end = charCodeEnd;
  microFonts[id].bitmap.tex_source = texSource;

  if (resName != NULL)
    microResourceStore(resName, RESOURCE_FONT, id);

  return id;
}

int microFontBitmapMake(const char *resName, int textureId, int charWidth,
                        int charHeight, int charCodeStart, int charCodeEnd)
{
  int tw, th;
  microTextureGetSize(textureId, &tw, &th);
  MicroTextureRegion texSource;
  texSource.x = 0;
  texSource.y = 0;
  texSource.w = tw;
  texSource.h = th;
  return microFontBitmapMakeFromPatch(resName, textureId, texSource, charWidth,
                                      charHeight, charCodeStart, charCodeEnd);
}

int microFontGetSize(int fontId)
{
  return microFonts[fontId].fontSize;
}

int microFontGetTextureId(int fontId)
{
  return microFonts[fontId].textureId;
}

int microFontGetLineHeight(int fontId, float scale)
{
  if (microFonts[fontId].type == MICRO_FONT_BITMAP)
    return microFonts[fontId].fontSize * scale;

  int ascent;
  stbtt_GetFontVMetrics(&microFonts[fontId].ttf.fontInfo, &ascent, 0, 0);
  return ascent * microFonts[fontId].fontSize * scale;
}

static int microFontTTFTextWidth(int fontId, const char *text, float scale)
{
  const stbtt_fontinfo *fontInfo = &microFonts[fontId].ttf.fontInfo;
  const float scale_px = stbtt_ScaleForPixelHeight(fontInfo, microFonts[fontId]
                                                               .fontSize) *
                         scale;

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
                      scale_px;
      lineWidth += kerning;
    }

    // Apply the left side bearing (lsb) and the advance of the character
    lineWidth += lsb * scale_px;
    lineWidth += (advance - lsb) * scale_px;

    prevChar = c;
  }

  // In case the string doesn't end with a newline, make sure we still update
  // totalWidth
  if (lineWidth > totalWidth)
    totalWidth = lineWidth;

  return totalWidth;
}

static int microFontBitmapTextWidth(int fontId, const char *text, float scale)
{
  float totalWidth = 0.0f;
  float lineWidth = 0.0f;

  for (const char *p = text; *p; ++p)
  {
    char c = *p;
    if (c == '\n')
    {
      // Reset for a new line
      if (lineWidth > totalWidth)
        totalWidth = lineWidth;
      lineWidth = 0.0f;
      continue;
    }

    lineWidth += microFonts[fontId].bitmap.char_width * scale;
  }

  // In case the string doesn't end with a newline, make sure we still update
  if (lineWidth > totalWidth)
    totalWidth = lineWidth;

  return totalWidth;
}

int microFontGetTextWidth(int fontId, const char *text, float scale)
{
  if (microFonts[fontId].type == MICRO_FONT_TTF)
    return microFontTTFTextWidth(fontId, text, scale);
  return microFontBitmapTextWidth(fontId, text, scale);
}

int microFontGetTextHeigth(int fontId, const char *text, float scale,
                           int lineSpacing)
{
  const int lineHeight = microFontGetLineHeight(fontId, scale);
  int linesCount = 1;
  for (const char *p = text; *p; ++p)
    if (*p == '\n')
      linesCount++;
  return linesCount * lineHeight + (linesCount - 1) * lineSpacing;
}

static void microFontTTFGetLinesWidth(int fontId, const char *text, float scale,
                                      int *widths, int *linesCount)
{
  const stbtt_fontinfo *fontInfo = &microFonts[fontId].ttf.fontInfo;
  const float scale_px = stbtt_ScaleForPixelHeight(fontInfo, microFonts[fontId]
                                                               .fontSize) *
                         scale;
  char prevChar = '\n';
  widths[0] = 0;
  *linesCount = 0;

  for (const char *p = text; *p; ++p)
  {
    char c = *p;

    if (c == '\n')
    {
      (*linesCount)++;
      widths[*linesCount] = 0;
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
                      scale_px;
      widths[*linesCount] += kerning;
    }

    // Apply the left side bearing (lsb) and the advance of the character
    widths[*linesCount] += lsb * scale_px;
    widths[*linesCount] += (advance - lsb) * scale_px;

    prevChar = c;
  }

  *linesCount += 1;
}

static void microFontBitmapGetLinesWidth(int fontId, const char *text,
                                         float scale, int maxLineWidth,
                                         int *widths, int *linesCount)
{
  int advance = microFonts[fontId].bitmap.char_width * scale;
  int i = 0;
  int textLen = strlen(text);
  widths[0] = 0;
  *linesCount = 0;

  for (i = 0; i < textLen; i++)
  {
    char c = text[i];

    // New line if width restriction is exceeded
    if (maxLineWidth)
    {

      if (c == ' ' && text[i + 1] != ' ')
      {
        int word_advance = advance;
        int j = 1;
        while (text[i + j] != ' ' && text[i + j] != '\0')
        {
          word_advance += advance;
          j++;
        }
        if ((widths[*linesCount] + word_advance) >= maxLineWidth)
        {
          *linesCount += 1;
          widths[*linesCount] = 0;
        }
      }

      if (widths[*linesCount] >= maxLineWidth)
      {
        *linesCount += 1;
        widths[*linesCount] = 0;
      }
    }

    if (c == '\n')
    {
      (*linesCount)++;
      widths[*linesCount] = 0;
      continue;
    }

    widths[*linesCount] += advance;
  }

  *linesCount += 1;
}

void microFontGetLinesWidth(int fontId, const char *text, float scale,
                            int maxLineWidth, int *widths, int *linesCount)
{
  if (microFonts[fontId].type == MICRO_FONT_TTF)
    microFontTTFGetLinesWidth(fontId, text, scale, widths, linesCount);
  else
    microFontBitmapGetLinesWidth(fontId, text, scale, maxLineWidth, widths,
                                 linesCount);
}

int microFontGet(const char *resName)
{
  return microResourceGet(RESOURCE_FONT, resName);
}

void microFontFree(int fontId)
{
  if (fontId < 0 || fontId >= MICRO_MAX_FONTS)
  {
    printf("Error: invalid font id %d\n", fontId);
    return;
  }

  if (microFonts[fontId].textureId == -1)
  {
    printf("Error: font %d is not loaded\n", fontId);
    return;
  }

  if (microFonts[fontId].type == MICRO_FONT_TTF)
  {
    microTextureFree(microFonts[fontId].textureId);
    free(microFonts[fontId].ttf.ttf_buffer);
  }
  microFonts[fontId].textureId = -1;
  microFontsCount--;

  microResourceRemove(RESOURCE_FONT, fontId);
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

int microShaderLoadFromFile(const char *resName, const char *vertexShaderPath,
                            const char *fragmentShaderPath)
{
  char vertShaderSrc[MICRO_MAX_SHADER_LEN];
  char fragShaderSrc[MICRO_MAX_SHADER_LEN];
  readShaderFile(vertexShaderPath, &vertShaderSrc[0]);
  readShaderFile(fragmentShaderPath, &fragShaderSrc[0]);
  return microShaderLoadFromSource(resName, &vertShaderSrc[0],
                                   &fragShaderSrc[0]);
}

int microShaderLoadFromSource(const char *resName, const char *vertexShaderSrc,
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
  assert(shader.uniformsCount < MICRO_MAX_UNIFORMS);
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

  // Load Attributes
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &shader.attributesCount);
  assert(shader.attributesCount < MICRO_MAX_ATTRIBUTES);
  for (int i = 0; i < shader.attributesCount; i++)
  {
    GLsizei length;
    GLint size;
    glGetActiveAttrib(program, (GLuint)i, MICRO_MAX_NAME_LEN, &length, &size,
                      &shader.attributesTypes[i], shader.attributesNames[i]);
    shader
      .attributesLocations[i] = glGetAttribLocation(program,
                                                    shader.attributesNames[i]);
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

  if (resName != NULL)
    microResourceStore(resName, RESOURCE_SHADER, spot);

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

int microShaderGetAttributeLocation(int shaderId, const char *name)
{
  assert(shaderId != -1);
  microShader *shader = &microShaders[shaderId];
  for (int i = 0; i < shader->attributesCount; i++)
  {
    if (strcmp(shader->attributesNames[i], name) == 0)
      return shader->attributesLocations[i];
  }
  return -1;
}

int microShaderGetUniformLocation(int shaderId, const char *name)
{
  assert(shaderId != -1);
  microShader *shader = &microShaders[shaderId];
  for (int i = 0; i < shader->uniformsCount; i++)
  {
    if (strcmp(shader->uniformsNames[i], name) == 0)
      return shader->uniformsLocations[i];
  }
  return -1;
}

int microShaderGet(const char *resName)
{
  return microResourceGet(RESOURCE_SHADER, resName);
}

void microShaderFree(int shaderId)
{
  if (shaderId == -1 || microShaders[shaderId].programId == (GLuint)-1)
    return;
  if (currentShader == shaderId)
    currentShader = -1;
  glDeleteProgram(microShaders[shaderId].programId);
  microShaders[shaderId].programId = -1;
  microResourceRemove(RESOURCE_SHADER, shaderId);
}

void microShaderApply(int shaderId)
{
  if (currentShader == shaderId)
    return;
  assert(shaderId >= 0 && shaderId < MICRO_MAX_SHADERS);
  glUseProgram(microShaders[shaderId].programId);
  currentShader = shaderId;
  debugInfo.shaderSwitches++;
}

void microShaderApplyDefault()
{
  microShaderApply(defaultShaderId);
}

void microShaderSetUniformf(const char *name, ...)
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
    abort_trace(); // Not implemented
    break;
  default:
    abort_trace(); // Unsupported uniform type
  }
}

void microShaderSetUniformi(const char *name, ...)
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
  case GL_INT:
    shader->uniformsValues[uniformId * 4] = (int)va_arg(args, int);
    glUniform1i(uniformLoc, shader->uniformsValues[uniformId * 4]);
    break;
  case GL_INT_VEC2:
    shader->uniformsValues[uniformId * 4] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 1] = (int)va_arg(args, int);
    glUniform2i(uniformLoc, shader->uniformsValues[uniformId * 4],
                shader->uniformsValues[uniformId * 4 + 1]);
    break;
  case GL_INT_VEC3:
    shader->uniformsValues[uniformId * 4] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 1] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 2] = (int)va_arg(args, int);
    glUniform3i(uniformLoc, shader->uniformsValues[uniformId * 4],
                shader->uniformsValues[uniformId * 4 + 1],
                shader->uniformsValues[uniformId * 4 + 2]);
    break;
  case GL_INT_VEC4:
    shader->uniformsValues[uniformId * 4] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 1] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 2] = (int)va_arg(args, int);
    shader->uniformsValues[uniformId * 4 + 3] = (int)va_arg(args, int);
    glUniform4i(uniformLoc, shader->uniformsValues[uniformId * 4],
                shader->uniformsValues[uniformId * 4 + 1],
                shader->uniformsValues[uniformId * 4 + 2],
                shader->uniformsValues[uniformId * 4 + 3]);
    break;
  case GL_SAMPLER_2D:
    shader->uniformsValues[uniformId * 4] = (int)va_arg(args, int);
    glUniform1i(uniformLoc, shader->uniformsValues[uniformId * 4]);
    break;
  default:
    debug_print("Unsupported uniform type: %d (%x)\n",
                shader->uniformsTypes[uniformId],
                shader->uniformsTypes[uniformId]);
    abort_trace(); // Unsupported uniform type
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
  abort_trace(); // Uniform not found
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
  abort_trace(); // Uniform not found
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
  abort_trace(); // Uniform not found
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
  abort_trace(); // Uniform not found
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

  // create texture with no data
  const int textureId = microTextureLoadFromMemory(NULL, 0, width, height, 4,
                                                   MICRO_FILTER_LINEAR, 0);
  assert(textureId != -1);
  assert(microTextures[textureId].id != 0);
  GL_CHECK_ERRORS();

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
  GL_CHECK_ERRORS();

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
// LIGHTING
///////////////////////////
int microLightAdd(float cx, float cy, float radius, float intensity)
{
  MicroLight light;
  light.is_active = true;
  light.cx = cx;
  light.cy = cy;
  light.radius = radius;
  light.intensity = intensity;
  light.color[0] = 255;
  light.color[1] = 255;
  light.color[2] = 255;

  // Find spot in the resources buffer
  int lightId = -1;
  if (microLightsDeletedCount)
  {
    lightId = microLightsDeleted[microLightsDeletedCount - 1];
    microLightsDeletedCount--;
  }
  else
  {
    lightId = microLightsCount;
    microLightsCount++;
  }
  assert(lightId < MICRO_MAX_LIGHTS);

  memcpy(&microLights[lightId], &light, sizeof(MicroLight));
  return lightId;
}

// Set light position
void microLightSetPosition(int lightId, float x, float y)
{
  microLights[lightId].cx = x;
  microLights[lightId].cy = y;
}

// Set light scale
void microLightSetRadius(int lightId, float radius)
{
  microLights[lightId].radius = radius;
}

// Set light intensity
void microLightSetIntensity(int lightId, float intensity)
{
  microLights[lightId].intensity = intensity;
}

void microLightSetActive(int lightId, int is_active)
{
  microLights[lightId].is_active = is_active;
}

// Remove light from scene
void microLightRemove(int lightId)
{
  if (lightId == microLightsCount - 1)
  {
    microLightsCount--;
    return;
  }
  microLights[lightId].is_active = false;
  microLightsDeleted[microLightsDeletedCount] = lightId;
  microLightsDeletedCount++;
}

// Remove all lights from scene
void microLightRemoveAll()
{
  for (int i = 0; i < microLightsCount; i++)
    microLights[i].is_active = 0;
  microLightsCount = 0;
  microLightsDeletedCount = 0;
}

void microLightsUpdateTexture()
{
  if (!microLightsCount)
    return;

  // Flush previous stuff
  microGraphicsDisplay();

  // SDL get winwdow size
  int w, h;
  microSystemGetWindowSize(&w, &h);

  microGraphicsRenderToCanvas(lightsCanvasId);
  glViewport(0, 0, w, h);
  microShaderApply(lightShaderId);
  MicroVAO *defaultVAO = &microVAOs[defaultVAOId];
  defaultVAO->shaderId = lightShaderId;
  glBlendFunc(GL_ONE, GL_ONE);
  // glBlendEquation(GL_FUNC_ADD);
  // glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
  glBlendEquation(GL_MIN);
  // glBlendEquationSeparate(GL_MAX, GL_MIN);
  microGraphicsClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  microGraphicsClear();
  microViewFlipY(true);
  microViewApply(lightShaderId);

  for (int i = 0; i < microLightsCount; i++)
  {
    const MicroLight *light = &microLights[i];

    if (!light->is_active)
      continue;

    microGraphicsDrawSprite(-1, 0, 0, 1, 1, light->cx, light->cy,
                            light->radius * 2.0, light->radius * 2.0,
                            light->radius, light->radius, 0.0, 0, 0, 0,
                            255 * light->intensity);
  }
  microGraphicsDisplay();
  microShaderApplyDefault();
  defaultVAO->shaderId = defaultShaderId;
  microGraphicsRenderToScreen();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  microViewFlipY(false);
  microViewApply(defaultShaderId);
}

int microLightsGetTextureId()
{
  return microCanvasGetTextureId(lightsCanvasId);
}

int microLightsGetCount()
{
  return microLightsCount;
}

int microLightsGetActiveCount()
{
  int count = 0;
  for (int i = 0; i < microLightsCount; i++)
    if (microLights[i].is_active)
      count++;
  return count;
}

void microLightsSetAmbientIntensity(float intensity)
{
  ambientLightIntensity = intensity;
}

float microLightsGetAmbientIntensity()
{
  return ambientLightIntensity;
}

////////////////////////////
// RENDERING
///////////////////////////
int microGraphicsInit()
{
#if defined(__linux__)
  GLenum err = glewInit();
  if (err != GLEW_OK)
    exit(1);
  else
    debug_print("GLEW version: %s\n", glewGetString(GLEW_VERSION));
#endif

  char *glVersion = (char *)glGetString(GL_VERSION);
  if (glVersion)
    debug_print("OpenGL version: %s\n", glVersion);

  // clear memory allocations
  for (int j = 0; j < RESOURCE_COUNT; j++)
  {
    for (int i = 0; i < MICRO_MAX_RESOURCES; i++)
    {
      resources_map[j][i].id = -1;
      resources_inv_map[j][i] = -1;
    }
  }
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
  microParticleEmitters = vec_new(sizeof(microParticleEmitter));
  microFreedParticleEmitters = vec_new(sizeof(int));

  // Load base shader
  defaultShaderId = microShaderLoadFromSource("micro_sprite_shader",
                                              baseVertexShaderSrc,
                                              baseFragmentShaderSrc);
  assert(defaultShaderId != -1);

  // Setup sprite buffers vector
  microVAOs = vec_new(sizeof(MicroVAO));

  // For 3d
  glEnable(GL_DEPTH_TEST);

  // Clear
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0, 0.0, 0.0, 0.0);

  // Enable Alpha
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GL_CHECK_ERRORS();

  // MICRO_VERTEX_BUFFER_SIZE
  int vbo_vp = microVBONew(2 * 6 * sizeof(float), MICRO_STREAM_DRAW,
                           sprites_vertex_buf);
  int vbo_p = microVBONew(2 * MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                          MICRO_STREAM_DRAW, NULL);
  int vbo_s = microVBONew(2 * MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                          MICRO_STREAM_DRAW, NULL);
  int vbo_o = microVBONew(2 * MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                          MICRO_STREAM_DRAW, NULL);
  int vbo_r = microVBONew(MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                          MICRO_STREAM_DRAW, NULL);
  int vbo_ts = microVBONew(4 * MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                           MICRO_STREAM_DRAW, NULL);
  int vbo_c = microVBONew(4 * MICRO_SPRITES_BUFFER_SIZE * sizeof(float),
                          MICRO_STREAM_DRAW, NULL);

  MicroAttributeData default_attrs[] = {
    {vbo_vp, true, "vpos", 2, MICRO_FLOAT, 0, 0, NULL, false},
    {vbo_p, true, "position", 2, MICRO_FLOAT, 0, 1, NULL, false},
    {vbo_s, true, "size", 2, MICRO_FLOAT, 0, 1, NULL, false},
    {vbo_o, true, "origin", 2, MICRO_FLOAT, 0, 1, NULL, false},
    {vbo_r, true, "rotation", 1, MICRO_FLOAT, 0, 1, NULL, false},
    {vbo_ts, true, "texcoord", 4, MICRO_FLOAT, 0, 1, NULL, false},
    {vbo_c, true, "color", 4, MICRO_UNSIGNED_BYTE, 0, 1, NULL, true},
  };

  defaultVAOId = microVAONew(defaultShaderId, -1, 6, 0, default_attrs,
                             sizeof(default_attrs) /
                               sizeof(MicroAttributeData));
  assert(defaultVAOId != -1);
  MicroVAO *defaultVAO = &microVAOs[defaultVAOId];
  defaultVAO->drawCmd.start = 0;
  defaultVAO->drawCmd.count = 6;
  defaultVAO->drawCmd.instanceCount = 0;
  defaultVAO->drawCmd.baseInstance = 0;

  // Load light shader
  lightShaderId = microShaderLoadFromSource("micro_light_shader",
                                            baseVertexShaderSrc,
                                            lightFragmentShaderSrc);
  assert(lightShaderId != -1);
  GL_CHECK_ERRORS();

  // Setup lights canvas
  int screenWidth, screenHeight;
  microSystemGetWindowSize(&screenWidth, &screenHeight);
  lightsCanvasId = microCanvasCreate(screenWidth, screenHeight);

  debug_print("microGraphics allocated %d kb\n",
              (int)((sizeof(microTextures) + sizeof(microCanvases) +
                     sizeof(microShaders) + sizeof(microAnimations) +
                     sizeof(microFonts) + sizeof(microLights)) /
                    (double)(1024)));

  // Clear debug info
  debugInfo.drawCalls = 0;
  debugInfo.vertices = 0;
  debugInfo.textureSwitches = 0;
  debugInfo.shaderSwitches = 0;
  debugInfo.bytesSent = 0;

  // Make sure screen is target of rendering
  microGraphicsRenderToScreen();

  return 0;
}

void microGraphicsQuit()
{
  // Delete default VAO
  microVAOFree(defaultVAOId);

  microAnimationFreeAll();
  microFontFreeAll();
  microParticleEmitterRemoveAll();

  vec_free(microParticleEmitters);
  vec_free(microFreedParticleEmitters);
  vec_free(microVAOs);
}

void microGraphicsClear()
{
  glClear(GL_COLOR_BUFFER_BIT);
}

void microGraphicsClearColor(float r, float g, float b, float a)
{
  glClearColor(r, g, b, a);
}

void microGraphicsRenderToScreen()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int windowWidth, windowHeight;
  microSystemGetWindowSize(&windowWidth, &windowHeight);
  glViewport(0, 0, windowWidth, windowHeight);
  GL_CHECK_ERRORS();
}

void microGraphicsRenderToCanvas(int canvasId)
{
  assert(canvasId >= 0 && canvasId < MICRO_MAX_CANVASES);
  glBindFramebuffer(GL_FRAMEBUFFER, microCanvases[canvasId].framebufferId);
  glViewport(0, 0, microCanvases[canvasId].width,
             microCanvases[canvasId].height);
  GL_CHECK_ERRORS();
}

void microGraphicsDisplay()
{
  MicroVAO *defaultVAO = &microVAOs[defaultVAOId];
  if (defaultVAO->drawCmd.instanceCount == 0)
    return;
  assert(defaultVAO->drawCmd.instanceCount <= MICRO_SPRITES_BUFFER_SIZE);

  // Set texture of VAO
  defaultVAO->textureGLId = currentTexture;

  // Stream data to GPU
  microVAOSubmit(defaultVAOId, "position", &sprites_pos_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);
  microVAOSubmit(defaultVAOId, "size", &sprites_size_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);
  microVAOSubmit(defaultVAOId, "origin", &sprites_origin_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);
  microVAOSubmit(defaultVAOId, "rotation", &sprites_rotation_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);
  microVAOSubmit(defaultVAOId, "texcoord", &sprites_texsrc_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);
  microVAOSubmit(defaultVAOId, "color", &sprites_color_buf[0], 0,
                 defaultVAO->drawCmd.instanceCount);

  // Draw the VAO
  microVAODraw(defaultVAOId);

  // Reset
  defaultVAO->drawCmd.instanceCount = 0;
}

int microVAONew(int shaderId, int textureId, int vertexCount,
                int instancesCount, const MicroAttributeData *attrs,
                int attributesCount)
{
  assert(attributesCount < MICRO_MAX_ATTRIBUTES);
  assert(shaderId != -1);

  MicroVAO vao;
  microShaderApply(shaderId);

  // Create VAO
  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);
  vao.vao_id = vao_id;
  glBindVertexArray(vao_id);
  GL_CHECK_ERRORS();
  vao.drawCmd.start = 0;
  vao.drawCmd.count = vertexCount;
  vao.drawCmd.instanceCount = instancesCount;
  vao.shaderId = shaderId;
  vao.textureGLId = textureId >= 0 ? microTextures[textureId].id : -1;
  vao.attr_count = attributesCount;
  vao.elements_bo = -1;

  glGenBuffers(1, &vao.draw_indirect_buf_id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vao.draw_indirect_buf_id);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawCommand), NULL,
               GL_DYNAMIC_DRAW);

  for (int i = 0; i < attributesCount; i++)
  {
    // Store attribute data in VAO for later
    memcpy(&vao.attrs[i], &attrs[i], sizeof(MicroAttributeData));

    glBindBuffer(GL_ARRAY_BUFFER, vao.attrs[i].vbo_id);
    GL_CHECK_ERRORS();
    int attr_loc = microShaderGetAttributeLocation(shaderId, attrs[i].name);
    assert(attr_loc != -1);
    if (attrs[i].type == MICRO_FLOAT || attrs[i].type == MICRO_DOUBLE ||
        attrs[i].normalized)
    {
      glVertexAttribPointer(attr_loc, attrs[i].components,
                            vao_component_types[attrs[i].type],
                            attrs[i].normalized, attrs[i].stride,
                            attrs[i].offset_bytes);
    }
    else
    {
      glVertexAttribIPointer(attr_loc, attrs[i].components,
                             vao_component_types[attrs[i].type],
                             attrs[i].stride, attrs[i].offset_bytes);
    }

    if (attrs[i].divisor > 0)
      glVertexAttribDivisor(attr_loc, attrs[i].divisor);
    glEnableVertexAttribArray(attr_loc);
    GL_CHECK_ERRORS();
  }

  // TODO: element buffer

  // Done with VAO setup
  glBindVertexArray(0);

  // Find a spot to store the VAO
  for (size_t i = 0; i < vec_len(microVAOs); i++)
  {
    MicroVAO *va = &microVAOs[i];
    if (va->vao_id != -1)
      continue;
    memcpy(va, &vao, sizeof(MicroVAO));
    return i;
  }
  vec_append(microVAOs, &vao);
  return vec_len(microVAOs) - 1;
}

void microVAOSubmit(int vaoId, const char *attribute_name, const void *data,
                    int start, int count)
{
  MicroVAO *vao = &microVAOs[vaoId];
  assert(vao->attr_count < MICRO_MAX_ATTRIBUTES);
  for (int i = 0; i < vao->attr_count; i++)
  {
    MicroAttributeData *attr = &vao->attrs[i];
    if (strcmp(attr->name, attribute_name) != 0)
      continue;
    microVBOSubmit(attr->vbo_id, data,
                   start * attr->components * vao_component_sizes[attr->type],
                   count * attr->components * vao_component_sizes[attr->type]);
    GL_CHECK_ERRORS_PRINT("vaoId %d vbo_id %d attr_name %s start %d count %d "
                          "components "
                          "%d component_size %d\n",
                          vaoId, attr->vbo_id, attribute_name,
                          start * attr->components *
                            vao_component_sizes[attr->type],
                          count * attr->components *
                            vao_component_sizes[attr->type],
                          attr->components, vao_component_sizes[attr->type]);
    return;
  }
  abort_trace(); // Attribute not found
}

void microVAODraw(int vaoId)
{
  MicroVAO *vao = &microVAOs[vaoId];
  if (vao->drawCmd.count == 0)
    return;

  // Statistic
  debugInfo.drawCalls++;
  debugInfo.vertices += vao->drawCmd.count * vao->drawCmd.instanceCount;

  // Finish drawing old stuff
  // if texture is different
  if (vao->textureGLId == -1)
  {
    if (currentTexture != 0)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(0, 0);
      GL_CHECK_ERRORS();
    }
  }
  else
  {
    if (currentTexture != vao->textureGLId)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(vao->textureGLId, 0);
      GL_CHECK_ERRORS();
    }
  }

  microShaderApply(vao->shaderId);
  glBindVertexArray(vao->vao_id);
  if (vao->drawCmd.instanceCount > 1)
  {
    if (vao->elements_bo != -1)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->elements_bo);
      glDrawElementsInstanced(GL_TRIANGLES, vao->drawCmd.count, GL_UNSIGNED_INT,
                              (void *)(vao->drawCmd.start *
                                       sizeof(unsigned int)),
                              1);
    }
    else
    {
      if (vao->drawCmd.baseInstance > 0)
      {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vao->draw_indirect_buf_id);
        glDrawArraysIndirect(GL_TRIANGLES, (void *)0);
      }
      else
      {
        glDrawArraysInstanced(GL_TRIANGLES, vao->drawCmd.start,
                              vao->drawCmd.count, vao->drawCmd.instanceCount);
      }
    }
  }
  else
  {
    if (vao->elements_bo != -1)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->elements_bo);
      glDrawElements(GL_TRIANGLES, vao->drawCmd.count, GL_UNSIGNED_INT,
                     (void *)(vao->drawCmd.start * sizeof(unsigned int)));
    }
    else
    {
      glDrawArrays(GL_TRIANGLES, vao->drawCmd.start, vao->drawCmd.count);
    }
  }
  GL_CHECK_ERRORS();
  glBindVertexArray(0);
}

void microVAOSetDrawRange(int vaoId, int start, int count, int instancesCount,
                          int baseInstance)
{
  MicroVAO *vao = &microVAOs[vaoId];
  vao->drawCmd.start = start;
  vao->drawCmd.count = count;
  vao->drawCmd.instanceCount = instancesCount;
  vao->drawCmd.baseInstance = baseInstance;
  if (instancesCount == 0)
    return;
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vao->draw_indirect_buf_id);
  glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(DrawCommand),
                  &vao->drawCmd);
}

void microVAOFree(int vaoId)
{
  MicroVAO *vao = &microVAOs[vaoId];
  GLuint vao_id = vao->vao_id;
  glDeleteVertexArrays(1, &vao_id);
  for (int i = 0; i < vao->attr_count; i++)
  {
    if (vao->attrs[i].consume_vbo && vao->attrs[i].vbo_id != -1)
    {
      GLuint vbo_id = vao->attrs[i].vbo_id;
      glDeleteBuffers(1, &vbo_id);
      vao->attrs[i].vbo_id = -1;
    }
  }
  if (vao->elements_bo != -1)
  {
    GLuint elements_bo = vao->elements_bo;
    glDeleteBuffers(1, &elements_bo);
    vao->elements_bo = -1;
  }
  glDeleteBuffers(1, &vao->draw_indirect_buf_id);
  GL_CHECK_ERRORS();
  vao->vao_id = -1;
  vao->drawCmd.count = 0;
}

unsigned int microVBONew(int size, MicroVAODrawType drawType, const void *data)
{
  GLuint vbo_id;
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, size, data, vao_draw_modes[drawType]);
  GL_CHECK_ERRORS();
  debugInfo.bytesSent += size;
  return vbo_id;
}
void microVBOSubmit(unsigned int vboId, const void *data, int start, int count)
{
  glBindBuffer(GL_ARRAY_BUFFER, vboId);
  glBufferSubData(GL_ARRAY_BUFFER, start, count, data);
  debugInfo.bytesSent += count;
}

void microVBOFree(unsigned int vboId)
{
  const GLuint vbo_id = vboId;
  glDeleteBuffers(1, &vbo_id);
  GL_CHECK_ERRORS();
}

void microGraphicsDrawSprite(int textureId, float tx, float ty, float tw,
                             float th, float x, float y, float w, float h,
                             float originX, float originY, float rotation,
                             unsigned char r, unsigned char g, unsigned char b,
                             unsigned char a)
{
  MicroVAO *defaultVAO = &microVAOs[defaultVAOId];
  int glTextureId = 0;
  if (textureId >= 0)
    glTextureId = microTextures[textureId].id;

  if (glTextureId == 0)
  {
    if (currentTexture != 0)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(0, 0);
    }
  }
  else
  {
    if (currentTexture != glTextureId)
    {
      microGraphicsDisplay();
      microGLStateBindTexture(glTextureId, 0);
    }
  }

  if (defaultVAO->drawCmd.instanceCount == MICRO_SPRITES_BUFFER_SIZE)
    microGraphicsDisplay();

  int tex_w = 1, tex_h = 1;
  if (textureId >= 0)
  {
    assert(microTextures[textureId].id != (GLuint)-1);
    microTextureGetSize(textureId, &tex_w, &tex_h);
  }
  const int is_id = defaultVAO->drawCmd.instanceCount;
  sprites_pos_buf[is_id * 2 + 0] = x;
  sprites_pos_buf[is_id * 2 + 1] = y;
  sprites_size_buf[is_id * 2 + 0] = w;
  sprites_size_buf[is_id * 2 + 1] = h;
  sprites_origin_buf[is_id * 2 + 0] = originX;
  sprites_origin_buf[is_id * 2 + 1] = originY;
  sprites_rotation_buf[is_id] = rotation;
  sprites_texsrc_buf[is_id * 4 + 0] = tx / (float)tex_w;
  sprites_texsrc_buf[is_id * 4 + 1] = ty / (float)tex_h;
  sprites_texsrc_buf[is_id * 4 + 2] = tw / (float)tex_w;
  sprites_texsrc_buf[is_id * 4 + 3] = th / (float)tex_h;
  sprites_color_buf[is_id * 4 + 0] = r;
  sprites_color_buf[is_id * 4 + 1] = g;
  sprites_color_buf[is_id * 4 + 2] = b;
  sprites_color_buf[is_id * 4 + 3] = a;

  defaultVAO->drawCmd.instanceCount++;
}

static void microGraphicsDrawTextTTF(int fontId, const char *text, float x,
                                     float y, float lineSpacing, float scale,
                                     TextAlignment align, int maxLineWidth,
                                     unsigned char r, unsigned char g,
                                     unsigned char b, unsigned char a)
{
  const unsigned int textLen = strlen(text);
  const stbtt_fontinfo *fontInfo = &microFonts[fontId].ttf.fontInfo;

  const float drawX = x;
  const float scale_px = stbtt_ScaleForPixelHeight(fontInfo,
                                                   microFonts[fontId].fontSize);
  // Load lines lenghts and count
  int linesCount = 1;
  int lineLengths[32];
  if (align == TEXT_ALIGN_RIGHT || align == TEXT_ALIGN_CENTER)
    microFontGetLinesWidth(fontId, text, scale, 0, lineLengths, &linesCount);
  unsigned int lineId = 0;

  if (align == TEXT_ALIGN_RIGHT)
    x = drawX - lineLengths[lineId];
  else if (align == TEXT_ALIGN_CENTER)
    x = drawX - lineLengths[lineId] / 2.f;
  else
    x = drawX;

  assert(isnan(scale_px) == 0);
  assert(isinf(scale_px) == 0);
  int ascent;
  int descent;
  stbtt_GetFontVMetrics(fontInfo, &ascent, &descent, 0);
  ascent *= scale_px * scale;

  char prevChar = '\n';
  for (unsigned int i = 0; i < textLen; i++)
  {
    const unsigned char c = text[i];

    // TODO: wrap words and lines if needed

    if (c == '\n')
    {
      y += ascent + lineSpacing * scale;
      lineId++;
      if (align == TEXT_ALIGN_RIGHT)
        x = drawX - lineLengths[lineId];
      else if (align == TEXT_ALIGN_CENTER)
        x = drawX - lineLengths[lineId] / 2.f;
      else
        x = drawX;
      prevChar = c;
      continue;
    }
    assert((int)c >= 32 && (int)c <= 126);

    // Get glyph metrics and position in texture atlas
    const int glyphIndex = microFonts[fontId].ttf.glyphsOffset[(int)c - 32];
    int advance, lsb, x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(fontInfo, c, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(fontInfo, c, scale_px, scale_px, &x0, &y0, &x1,
                                &y1);

    // Compute glyph texture coordinates
    const float tx = glyphIndex % MICRO_FONT_TEXTURE_SIZE;
    const float ty = (int)(glyphIndex / MICRO_FONT_TEXTURE_SIZE);
    const float tw = x1 - x0;
    const float th = y1 - y0;

    // Added kerning adjustment
    if (prevChar != '\n')
    {
      float kerning = stbtt_GetCodepointKernAdvance(fontInfo, prevChar, c) *
                      scale_px * scale;
      assert(isnan(kerning) == 0);
      x += kerning;
    }

    // Apply the left side bearing to the next character
    if (prevChar != '\n')
      x += lsb * scale_px * scale;

    // Draw glyph
    microGraphicsDrawSprite(microFontGetTextureId(fontId), tx, ty, tw, th, x,
                            y + y0 * scale, tw * scale, th * scale, 0.0, 0.0,
                            0.0, r, g, b, a);

    x += (advance - lsb) * scale_px * scale;
    prevChar = c;
  }
}

void microGraphicsDrawTextBitmap(int fontId, const char *text, float x, float y,
                                 float lineSpacing, float scale,
                                 TextAlignment align, int maxLineWidth,
                                 unsigned char r, unsigned char g,
                                 unsigned char b, unsigned char a)
{
  const unsigned int textLen = strlen(text);
  MicroFont *font = &microFonts[fontId];
  MicroFontBitmap *font_bm = &font->bitmap;

  const float drawX = x;

  // Load lines lenghts and count
  int linesCount = 1;
  int lineLengths[32];
  if (align == TEXT_ALIGN_RIGHT || align == TEXT_ALIGN_CENTER)
    microFontGetLinesWidth(fontId, text, scale, maxLineWidth, lineLengths,
                           &linesCount);
  unsigned int lineId = 0;
  int lineWidth = 0;

  if (align == TEXT_ALIGN_RIGHT)
    x = drawX - lineLengths[lineId];
  else if (align == TEXT_ALIGN_CENTER)
    x = drawX - lineLengths[lineId] / 2.f;
  else
    x = drawX;

  const int ascent = font->fontSize * scale;
  const int advance = font_bm->char_width * scale;
  for (unsigned int i = 0; i < textLen; i++)
  {
    const unsigned char c = text[i];

    // New line if width restriction is exceeded
    if (maxLineWidth)
    {

      if (c == ' ' && text[i + 1] != ' ')
      {
        int word_advance = advance;
        int j = 1;
        while (text[i + j] != ' ' && text[i + j] != '\0')
        {
          word_advance += advance;
          j++;
        }
        if (lineWidth + word_advance >= maxLineWidth)
        {
          y += ascent + lineSpacing * scale;
          lineId++;
          lineWidth = 0;
          if (align == TEXT_ALIGN_RIGHT)
            x = drawX - lineLengths[lineId];
          else if (align == TEXT_ALIGN_CENTER)
            x = drawX - lineLengths[lineId] / 2.f;
          else
            x = drawX;
          continue;
        }
      }

      if (lineWidth + advance >= maxLineWidth)
      {
        y += ascent + lineSpacing * scale;
        lineId++;
        lineWidth = 0;
        if (align == TEXT_ALIGN_RIGHT)
          x = drawX - lineLengths[lineId];
        else if (align == TEXT_ALIGN_CENTER)
          x = drawX - lineLengths[lineId] / 2.f;
        else
          x = drawX;
      }
    }

    if (c == '\n')
    {
      y += ascent + lineSpacing * scale;
      lineId++;
      lineWidth = 0;
      if (align == TEXT_ALIGN_RIGHT)
        x = drawX - lineLengths[lineId];
      else if (align == TEXT_ALIGN_CENTER)
        x = drawX - lineLengths[lineId] / 2.f;
      else
        x = drawX;
      continue;
    }

    // Draw only if its not a space and it is valid character
    if (c != ' ' && ((int)c >= font_bm->char_code_start &&
                     (int)c <= font_bm->char_code_end))
    {

      int glyphIndex = (int)c - font_bm->char_code_start;
      const float tx = (glyphIndex * font_bm->char_width) %
                         font_bm->tex_source.w +
                       font_bm->tex_source.x;
      const float ty = floor((glyphIndex * font_bm->char_width) /
                             (float)font_bm->tex_source.w) *
                         font->fontSize +
                       font_bm->tex_source.y;
      const float tw = font_bm->char_width;
      const float th = font->fontSize;

      // Draw glyph
      microGraphicsDrawSprite(font->textureId, tx, ty, tw, th, x, y, tw * scale,
                              th * scale, 0.0, 0.0, 0.0, r, g, b, a);
    }

    x += advance;
    lineWidth += advance;
  }
}

void microGraphicsDrawText(int fontId, const char *text, float x, float y,
                           float lineSpacing, float scale, TextAlignment align,
                           int maxLineWidth, unsigned char r, unsigned char g,
                           unsigned char b, unsigned char a)
{
  if (microFonts[fontId].type == MICRO_FONT_BITMAP)
    microGraphicsDrawTextBitmap(fontId, text, x, y, lineSpacing, scale, align,
                                maxLineWidth, r, g, b, a);
  else
    microGraphicsDrawTextTTF(fontId, text, x, y, lineSpacing, scale, align,
                             maxLineWidth, r, g, b, a);
}

int microGraphicsGetSpriteShaderId()
{
  return defaultShaderId;
}

static void proj_perspective_gl_rh(Mat4 m, float fovY_deg, float aspect,
                                   float zn, float zf, int flipY)
{
  mat4_identity(m);
  float f = 1.0f / tanf(0.5f * fovY_deg * (float)M_PI / 180.0f);
  m[MIDX(0, 0)] = f / (aspect > 0 ? aspect : 1.0f);
  m[MIDX(1, 1)] = f;
  m[MIDX(2, 2)] = (zf + zn) / (zn - zf);
  m[MIDX(2, 3)] = -1.0f;
  m[MIDX(3, 2)] = (2.0f * zf * zn) / (zn - zf);
  m[MIDX(3, 3)] = 0.0f;
  if (flipY)
    m[MIDX(1, 1)] = -m[MIDX(1, 1)];
}

static void proj_ortho_gl_rh(Mat4 m, float width, float height, float zn,
                             float zf, int flipY)
{
  mat4_identity(m);
  float l = -0.5f * width, r = 0.5f * width;
  float b = -0.5f * height, t = 0.5f * height;
  m[MIDX(0, 0)] = 2.0f / (r - l);
  m[MIDX(1, 1)] = 2.0f / (t - b);
  m[MIDX(2, 2)] = -2.0f / (zf - zn);
  m[MIDX(3, 0)] = -(r + l) / (r - l);
  m[MIDX(3, 1)] = -(t + b) / (t - b);
  m[MIDX(3, 2)] = -(zf + zn) / (zf - zn);
  if (flipY)
    m[MIDX(1, 1)] = -m[MIDX(1, 1)];
}

// VIEW
void microViewSet(MicroView new_view)
{
  view.viewportX = new_view.viewportX;
  view.viewportY = new_view.viewportY;
  view.viewportWidth = new_view.viewportWidth;
  view.viewportHeight = new_view.viewportHeight;
  view.centerX = new_view.centerX;
  view.centerY = new_view.centerY;
  view.width = new_view.width;
  view.height = new_view.height;
  view.rotation = new_view.rotation;
  view.flipY = (new_view.flipY > 0);
  view._need_matrix_update = true;
}

const MicroView *microViewGet()
{
  return &view;
}

void microViewApply(int shaderId)
{
  if (view._need_matrix_update)
  {
    float NCenterX, NCenterY;
    NCenterX = floor(view.centerX);
    NCenterY = floor(view.centerY);

    // Rotation components
    float angle = view.rotation;
    float cosine = cosf(angle);
    float sine = sinf(angle);
    float tx = -NCenterX * cosine - NCenterY * sine + NCenterX;
    float ty = NCenterX * sine - NCenterY * cosine + NCenterY;

    // Projection components
    float a = 2.f / view.width;
    float b = view.flipY
                ? 2.f / view.height
                : -2.f / view.height; // Change the sign of b based on flipY
    float c = -a * NCenterX;
    float d = -b * NCenterY;

    const float zNear = 0;
    const float zFar = 10;
    const float e = 2.f / (zFar - zNear);
    const float f = -(zFar + zNear) / (zFar - zNear);

    // Update matrix
    view.viewProj[0] = a * cosine;
    view.viewProj[1] = -b * sine;
    view.viewProj[2] = 0.f;
    view.viewProj[3] = 0.f;
    view.viewProj[4] = a * sine;
    view.viewProj[5] = b * cosine;
    view.viewProj[6] = 0.f;
    view.viewProj[7] = 0.f;
    view.viewProj[8] = 0.f;
    view.viewProj[9] = 0.f;
    view.viewProj[10] = e;
    view.viewProj[11] = 0.f;
    view.viewProj[12] = a * tx + c;
    view.viewProj[13] = b * ty + d;
    view.viewProj[14] = f;
    view.viewProj[15] = 1.f;

    // Set viewport
    glViewport(view.viewportX, view.viewportY, view.viewportWidth,
               view.viewportHeight);

    view._need_matrix_update = false;
  }

  // Apply view
  microShaderApply(shaderId);
  microShaderSetMatrix4("u_view", view.viewProj);
}

void microViewFlipY(bool flipY)
{
  view.flipY = flipY;
  view._need_matrix_update = true;
}

void microViewSetViewport(float x, float y, float width, float height)
{
  view.viewportX = x;
  view.viewportY = y;
  view.viewportWidth = width;
  view.viewportHeight = height;
  view._need_matrix_update = true;
}

void microViewSetCenter(float x, float y)
{
  view.centerX = x;
  view.centerY = y;
  view._need_matrix_update = true;
}

void microViewSetSize(float width, float height)
{
  view.width = width;
  view.height = height;
  view._need_matrix_update = true;
}

void microViewSetRotation(float rotation)
{
  view.rotation = rotation;
  view._need_matrix_update = true;
}

void microViewGetCenter(float *centerX, float *centerY)
{
  *centerX = view.centerX;
  *centerY = view.centerY;
}

void microViewGetSize(float *width, float *height)
{
  *width = view.width;
  *height = view.height;
}

float microViewGetRotation()
{
  return view.rotation;
}

void microViewGetViewport(float *width, float *height)
{
  *width = view.viewportWidth;
  *height = view.viewportHeight;
}

void microViewPointWorldToScreen(float x, float y, float *outX, float *outY)
{
  // 1. Translate to the view's local coordinate system (center becomes
  // origin)
  x -= view.centerX;
  y -= view.centerY;

  // 2. Rotate around the (new) origin
  const float angle = -view.rotation;
  const float cosine = cosf(angle);
  const float sine = sinf(angle);
  float tx = x * cosine - y * sine;
  float ty = x * sine + y * cosine;

  // 3. Scale (if required)
  const float scaleX = view.viewportWidth / view.width;
  const float scaleY = view.viewportHeight / view.height;
  tx *= scaleX;
  ty *= scaleY;

  // 4. Translate to screen coordinates (from the origin to the center of the
  // viewport)
  *outX = tx + view.viewportWidth / 2;
  *outY = ty + view.viewportHeight / 2;
}

void microViewPointScreenToWorld(float x, float y, float *outX, float *outY)
{
  // 1. Inverse Translation to the origin
  x -= view.viewportWidth / 2;
  y -= view.viewportHeight / 2;

  // 2. Inverse Scaling
  const float invScaleX = view.width / view.viewportWidth;
  const float invScaleY = view.height / view.viewportHeight;
  x *= invScaleX;
  y *= invScaleY;

  // 3. Inverse Rotation
  const float angle = -view.rotation;
  const float cosine = cosf(angle);
  const float sine = sinf(angle);
  float tx = x * cosine +
             y * sine; // Note: inverse rotation uses + for sine in x
  float ty = -x * sine +
             y * cosine; // Note: inverse rotation uses - for sine in y

  // 4. Inverse centering translation
  *outX = tx + view.centerX;
  *outY = ty + view.centerY;
}

void microView3dSet(MicroView3d view)
{
  view3d = view;
  view3d._need_update_view = true;
  view3d._need_update_proj = true;
}

const MicroView3d *microView3dGet()
{
  return &view3d;
}

void microView3dApply(int shaderId)
{
  const bool viewProjNeedsUpdate = view3d._need_update_view ||
                                   view3d._need_update_proj;
  // VIEW
  if (view3d._need_update_view)
  {
    // Camera basis (columns = +X,+Y,+Z in world)
    Vec3 xcol, ycol, zcol;
    quat_to_mat3_cols(view3d.orientation, xcol, ycol, zcol);

    const float *p = view3d.position;
    float *V = view3d.view;

    // R^T (rows are the camera axes in world)
    V[MIDX(0, 0)] = xcol[0];
    V[MIDX(0, 1)] = xcol[1];
    V[MIDX(0, 2)] = xcol[2];
    V[MIDX(1, 0)] = ycol[0];
    V[MIDX(1, 1)] = ycol[1];
    V[MIDX(1, 2)] = ycol[2];
    V[MIDX(2, 0)] = zcol[0];
    V[MIDX(2, 1)] = zcol[1];
    V[MIDX(2, 2)] = zcol[2];

    // Last column = -R^T * pos
    V[MIDX(0, 3)] = -(xcol[0] * p[0] + xcol[1] * p[1] + xcol[2] * p[2]);
    V[MIDX(1, 3)] = -(ycol[0] * p[0] + ycol[1] * p[1] + ycol[2] * p[2]);
    V[MIDX(2, 3)] = -(zcol[0] * p[0] + zcol[1] * p[1] + zcol[2] * p[2]);

    // Bottom row
    V[MIDX(3, 0)] = 0.0f;
    V[MIDX(3, 1)] = 0.0f;
    V[MIDX(3, 2)] = 0.0f;
    V[MIDX(3, 3)] = 1.0f;

    view3d._need_update_view = false;
  }

  // PROJECTION
  if (view3d._need_update_proj)
  {
    float aspect = (view3d.viewportHeight > 0.0f)
                     ? (view3d.viewportWidth / view3d.viewportHeight)
                     : 1.0f;
    if (view3d.projectionType == VIEW_PERSPECTIVE)
      proj_perspective_gl_rh(view3d.proj, view3d.fovY, aspect, view3d.nearZ,
                             view3d.farZ, view3d.flipY);
    else
      proj_ortho_gl_rh(view3d.proj, view3d.orthoWidth, view3d.orthoHeight,
                       view3d.nearZ, view3d.farZ, view3d.flipY);

    // after computing viewProj, before drawing
    glViewport((GLint)view3d.viewportX, (GLint)view3d.viewportY,
               (GLsizei)view3d.viewportWidth, (GLsizei)view3d.viewportHeight);

    view3d._need_update_proj = false;
  }

  // VIEW PROJECTION
  if (viewProjNeedsUpdate)
    mat4_mul(view3d.viewProj, view3d.proj, view3d.view);
  printf("view3d matrix:\n");
  mat4_print(view3d.viewProj);

  // Apply view
  const int tmp_shader_id = currentShader;
  microShaderApply(shaderId);
  microShaderSetMatrix4("u_view", view3d.viewProj);
  microShaderApply(tmp_shader_id);
}

void microView3dSetPosition(float x, float y, float z)
{
  view3d.position[0] = x;
  view3d.position[1] = y;
  view3d.position[2] = z;
  view3d._need_update_view = true;
}

void microView3dSetOrientation(float x, float y, float z, float w)
{
  view3d.orientation[0] = x;
  view3d.orientation[1] = y;
  view3d.orientation[2] = z;
  view3d.orientation[3] = w;
  view3d._need_update_view = true;
}

void microView3dLookAt(float eyeX, float eyeY, float eyeZ, float targetX,
                       float targetY, float targetZ, float upX, float upY,
                       float upZ)
{
  vec3_set(view3d.position, eyeX, eyeY, eyeZ);
  float eye[3] = {eyeX, eyeY, eyeZ}, tgt[3] = {targetX, targetY, targetZ},
        up[3] = {upX, upY, upZ};
  float fwd[3];
  vec3_sub(fwd, tgt, eye);
  vec3_norm(fwd, fwd); // world forward (where camera looks)
  float zcol[3] = {-fwd[0], -fwd[1], -fwd[2]}; // camera -Z in world
  float xcol[3];
  vec3_cross(xcol, up, zcol);
  vec3_norm(xcol, xcol); // camera +X in world
  float ycol[3];
  vec3_cross(ycol, zcol, xcol); // camera +Y in world
  quat_from_rotation_matrix_cols(view3d.orientation, xcol, ycol, zcol);
  view3d._need_update_view = true;
}

void microView3dSetPerspective(float fovY_deg, float nearZ, float farZ)
{
  view3d.fovY = fovY_deg;
  view3d.nearZ = nearZ;
  view3d.farZ = farZ;
  view3d.projectionType = VIEW_PERSPECTIVE;
  view3d._need_update_proj = true;
}

void microView3dSetOrthographic(float width, float height, float nearZ,
                                float farZ)
{
  view3d.orthoWidth = width;
  view3d.orthoHeight = height;
  view3d.nearZ = nearZ;
  view3d.farZ = farZ;
  view3d.projectionType = VIEW_ORTHOGRAPHIC;
  view3d._need_update_proj = true;
}

void microView3dFlyMoveLocal(float dx, float dy, float dz)
{
  float xcol[3], ycol[3], zcol[3];
  quat_to_mat3_cols(view3d.orientation, xcol, ycol, zcol);
  float move[3] = {xcol[0] * dx + ycol[0] * dy + (-zcol[0]) * dz,
                   xcol[1] * dx + ycol[1] * dy + (-zcol[1]) * dz,
                   xcol[2] * dx + ycol[2] * dy + (-zcol[2]) * dz};
  view3d.position[0] += move[0];
  view3d.position[1] += move[1];
  view3d.position[2] += move[2];
  view3d._need_update_view = true;
}

void microView3dFlyRotate(float dYaw, float dPitch, float dRoll)
{
  // yaw about world +Y
  if (dYaw != 0.0f)
  {
    float axisY[3] = {0, 1, 0}, qy[4];
    quat_from_axis_angle(qy, axisY, dYaw);
    float out[4];
    quat_mul(out, qy, view3d.orientation);
    memcpy(view3d.orientation, out, sizeof(out));
  }
  // pitch about camera local +X
  if (dPitch != 0.0f)
  {
    float xcol[3], ycol[3], zcol[3];
    quat_to_mat3_cols(view3d.orientation, xcol, ycol, zcol);
    float qx[4];
    quat_from_axis_angle(qx, xcol, dPitch);
    float out[4];
    quat_mul(out, qx, view3d.orientation);
    memcpy(view3d.orientation, out, sizeof(out));
  }
  // roll about camera local -Z forward axis
  if (dRoll != 0.0f)
  {
    float xcol[3], ycol[3], zcol[3];
    quat_to_mat3_cols(view3d.orientation, xcol, ycol, zcol);
    float fwd[3] = {-zcol[0], -zcol[1], -zcol[2]};
    float qz[4];
    quat_from_axis_angle(qz, fwd, dRoll);
    float out[4];
    quat_mul(out, qz, view3d.orientation);
    memcpy(view3d.orientation, out, sizeof(out));
  }
  quat_normalize(view3d.orientation);
  view3d._need_update_view = true;
}

void microView3dSetViewport(float x, float y, float width, float height)
{
  view3d.viewportX = x;
  view3d.viewportY = y;
  view3d.viewportWidth = width;
  view3d.viewportHeight = height;
  view3d._need_update_proj = true;
}

RenderingDebugInfo microGetRenderingDebugInfo()
{
  return debugInfo;
}

void microRenderingDebugInfoClear()
{
  // Clear debug info
  debugInfo.drawCalls = 0;
  debugInfo.vertices = 0;
  debugInfo.textureSwitches = 0;
  debugInfo.shaderSwitches = 0;
  debugInfo.bytesSent = 0;
}

static uint64_t lastTime = 0;
static uint64_t frequency = 0;

// TODO: fix
#include <SDL2/SDL.h>
float microGraphicsDelayToNextFrame(float target_fps)
{
  if (frequency == 0)
  {
    frequency = SDL_GetPerformanceFrequency();
    lastTime = SDL_GetPerformanceCounter();
  }

  uint64_t currentTime = SDL_GetPerformanceCounter();
  float frame_time = (float)(currentTime - lastTime) / frequency;

  // If frame finished early
  float target_frame_time = 1.0f / target_fps;
  if (target_frame_time > frame_time)
  {
    // Calculate remaining time in milliseconds and delay
    uint32_t delay_ms = (uint32_t)((target_frame_time - frame_time) * 1000.0f);
    if (delay_ms > 0)
    {
      SDL_Delay(delay_ms);
    }

    // Update current time after delay
    currentTime = SDL_GetPerformanceCounter();
    frame_time = (float)(currentTime - lastTime) / frequency;
  }

  lastTime = currentTime;

  // Clamp deltaTime to avoid huge spikes
  frame_time = fminf(frame_time, 2.0f / target_fps);

  return frame_time;
}

////////////////////////////
/// Particle emitter
////////////////////////////
int microParticleEmitterCreateSteady(int x, int y, float emissionRate,
                                     MicroParticle (*generationFunc)(int))
{
  // Create emitter
  microParticleEmitter newEmitter;
  newEmitter.particles = vec_new(sizeof(MicroParticle));
  newEmitter.freeParticles = vec_new(sizeof(int));
  newEmitter.particleGenerator = generationFunc;
  newEmitter.emitterType = MICRO_EMITTER_STEADY;
  newEmitter.emissionTimer = 0;
  newEmitter.emissionRate = emissionRate;
  newEmitter.width = 1;
  newEmitter.height = 1;
  newEmitter.x = x;
  newEmitter.y = y;

  int spot = -1;
  if (vec_len(microFreedParticleEmitters) > 0)
  {
    // Reuse a freed emitter
    spot = *(int *)vec_back(microFreedParticleEmitters);
    vec_pop_back(microFreedParticleEmitters);
    memcpy(&microParticleEmitters[spot], &newEmitter,
           sizeof(microParticleEmitter));
  }
  else
  {
    vec_append(microParticleEmitters, &newEmitter);
    spot = vec_len(microParticleEmitters) - 1;
  }

  return spot;
}

int microParticleEmitterCreateExplosion(int x, int y, int particlesCount,
                                        MicroParticle (*generationFunc)(int))
{
  // Create emitter
  microParticleEmitter newEmitter;
  newEmitter.particles = vec_new(sizeof(MicroParticle));
  newEmitter.freeParticles = vec_new(sizeof(int));
  newEmitter.particleGenerator = generationFunc;
  newEmitter.emitterType = MICRO_EMITTER_EXPLOSION;
  newEmitter.emissionTimer = 0;
  newEmitter.emissionRate = 0;
  newEmitter.width = 1;
  newEmitter.height = 1;
  newEmitter.x = x;
  newEmitter.y = y;

  int spot = -1;
  if (vec_len(microFreedParticleEmitters) > 0)
  {
    // Reuse a freed emitter
    spot = *(int *)vec_back(microFreedParticleEmitters);
    vec_pop_back(microFreedParticleEmitters);
    memcpy(&microParticleEmitters[spot], &newEmitter,
           sizeof(microParticleEmitter));
  }
  else
  {
    vec_append(microParticleEmitters, &newEmitter);
    spot = vec_len(microParticleEmitters) - 1;
  }
  microParticleEmitter *emitter = &microParticleEmitters[spot];

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
    vec_append(emitter->particles, &p);
    microTotalParticles++;
  }

  return spot;
}

void microParticleEmitterSetPosition(int emitterId, int x, int y)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  emitter->x = x;
  emitter->y = y;
}

void microParticleEmitterSetSize(int emitterId, int width, int height)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  emitter->width = width;
  emitter->height = height;
}

void microParticleEmitterSetEmissionRate(int emitterId, float emissionRate)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  emitter->emissionRate = emissionRate;
}

void microParticleEmitterSetGenerationFunc(int emitterId,
                                           MicroParticle (*generationFunc)(int))
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  emitter->particleGenerator = generationFunc;
}
void microParticleEmitterGetPosition(int emitterId, int *x, int *y)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  *x = emitter->x;
  *y = emitter->y;
}

float microParticleEmitterGetEmissionRate(int emitterId)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  return emitter->emissionRate;
}

void microParticleEmittersUpdate(float dt)
{
  for (size_t i = 0; i < vec_len(microParticleEmitters); i++)
  {

    microParticleEmitter *emitter = &microParticleEmitters[i];
    MicroParticle *particles = emitter->particles;

    // Update particles
    for (unsigned int j = 0; j < vec_len(particles); j++)
    {
      MicroParticle *p = &particles[j];
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
    for (unsigned int j = 0; j < vec_len(particles); j++)
    {
      MicroParticle *p = &particles[j];
      if (p->life <= 0 && p->alive)
      {
        p->alive = 0;
        vec_append(emitter->freeParticles, &j);
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

        if (vec_len(emitter->freeParticles) > 0)
        {
          int spot = *(int *)vec_back(emitter->freeParticles);
          vec_pop_back(emitter->freeParticles);
          memcpy(&particles[spot], &p, sizeof(MicroParticle));
        }
        else
        {
          vec_append(particles, &p);
        }

        emitter->emissionTimer = 0;
      }
    }
  }
}

void microParticleEmitterDraw(int emitterId)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  MicroParticle **particles = &emitter->particles;

  for (unsigned int i = 0; i < vec_len(particles); i++)
  {
    MicroParticle *p = particles[i];
    if (p->life <= 0.0)
      continue;

    microGraphicsDrawSprite(p->textureId, p->tx, p->ty, p->tw, p->th, p->x,
                            p->y, p->scale, p->scale, p->scale / 2,
                            p->scale / 2, p->rotation, 255, 255, 255,
                            255 * p->alpha);
  }
}

void microParticleEmitterRemove(int emitterId)
{
  microParticleEmitter *emitter = &microParticleEmitters[emitterId];
  vec_free(emitter->particles);
  vec_free(emitter->freeParticles);
  vec_append(microFreedParticleEmitters, &emitterId);
}

void microParticleEmitterRemoveAll()
{
  for (unsigned int i = 0; i < vec_len(microParticleEmitters); i++)
    microParticleEmitterRemove(i);
}

int microParticlesCount()
{
  return microTotalParticles;
}
