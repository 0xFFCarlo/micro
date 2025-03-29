#include "Resources.h"
#include "../util/debug.h"
#include "Audio.h"
#include "Graphics.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define MICRO_RESOURCES_MAX 512
#define MICRO_RESOURCES_NAME_MAX_LEN 32
#define MICRO_TYPE_TEXTURE 0
#define MICRO_TYPE_SOUND 1
#define MICRO_TYPE_ATLAS 2
#define MICRO_TYPE_FONT 3

typedef struct Resource
{
  char name[MICRO_RESOURCES_NAME_MAX_LEN];
  int resourceType;
  int resourceId;
} Resource;

static Resource resources[MICRO_RESOURCES_MAX];
static int resourcesCount = 0;

#define CMP_EQUAL 0
int strcompare(const char *stra, const char *strb)
{
  int i = 0;
  while (i < MICRO_RESOURCES_NAME_MAX_LEN)
  {
    if (stra[i] == '\0' || strb[i] == '\0')
    {
      if (strb[i] == stra[i])
        return 0;
      else if (stra[i] == '\0')
        return -1;
      else
        return 1;
    }
    if (stra[i] > strb[i])
      return 1;
    else if (stra[i] < strb[i])
      return -1;
    i++;
  }
  return -1;
}

int microResourceGetKeyFromName(const char *name)
{
  // Binary search
  int min = 0;
  int max = resourcesCount - 1;
  int mid = (min + max) / 2;
  while (min <= max)
  {
    mid = (min + max) / 2;
    int cmp = strcompare(name, resources[mid].name);
    if (cmp == CMP_EQUAL)
      return mid;
    else if (cmp == -1)
      max = mid - 1;
    else
      min = mid + 1;
  }
  return -1;
}

int microResourceLoad(const char *name, const char *filepath, const char *type)
{
  assert(resourcesCount + 1 < MICRO_RESOURCES_MAX);

  int resourceId = -1;
  int resourceType = -1;
  if (strcompare(type, "texture") == CMP_EQUAL)
  {
    resourceId = microTextureLoadFromFile(name, filepath);
    if (resourceId == -1) {
      debug_print("Error: could not load texture at %s\n", filepath);
      abort();
    }
    resourceType = MICRO_TYPE_TEXTURE;
  }
  else if (strcompare(type, "atlas") == CMP_EQUAL)
  {
    resourceId = microAtlasLoadFromPath(name, filepath);
    if (resourceId == -1) {
      debug_print("Error: could not load atlas at %s\n", filepath);
      abort();
    }
    resourceType = MICRO_TYPE_ATLAS;
  }
  else if (strcompare(type, "sound") == CMP_EQUAL)
  {
    resourceId = microSoundLoadFromFile(filepath);
    if (resourceId == -1) {
      debug_print("Error: could not load sound at %s\n", filepath);
      abort();
    }
    resourceType = MICRO_TYPE_SOUND;
  }
  else
  {
    debug_print("Error: resource type %s not supported\n", type);
    return -1;
  }

  // store resource sorted by name in resources array
  int i = resourcesCount;
  while (i > 0 && strcompare(name, resources[i - 1].name) == -1)
  {
    resources[i] = resources[i - 1];
    i--;
  }
  resources[i].resourceId = resourceId;
  resources[i].resourceType = resourceType;
  strcpy(resources[i].name, name);
  resourcesCount++;
  assert(resourcesCount < MICRO_RESOURCES_MAX);

  return resourceId;
}

const char *get_file_extension(const char *filename)
{
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

const char *get_resource_type_from_ext(const char *ext)
{
  if (strcompare(ext, "png") == CMP_EQUAL ||
      strcompare(ext, "jpeg") == CMP_EQUAL ||
      strcompare(ext, "jpg") == CMP_EQUAL ||
      strcompare(ext, "bmp") == CMP_EQUAL)
    return "texture";
  if (strcompare(ext, "wav") == CMP_EQUAL ||
      strcompare(ext, "mp3") == CMP_EQUAL)
    return "sound";
  if (strcompare(ext, "ttf") == CMP_EQUAL)
    return "font";
  return "";
}

// extract file name without extension
char *get_file_name(const char *filename)
{
  char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  int len = strlen(filename) - strlen(dot);
  char *name = malloc(len + 1);
  strncpy(name, filename, len);
  name[len] = '\0';
  return name;
}

int microResourceAutoLoad(const char *dirpath)
{
  DIR *d;
  struct dirent *dir;
  d = opendir(dirpath);
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if (dir->d_type == DT_REG)
      {
        const char *format = get_file_extension(dir->d_name);
        const char *resource_type = get_resource_type_from_ext(format);
        if (strcompare(resource_type, "") == CMP_EQUAL)
        {
          debug_print("Error: resource type not supported for file %s\n",
                      dir->d_name);
          continue;
        }
        char *name = get_file_name(dir->d_name);
        char filepath[256];
        sprintf(filepath, "%s%s", dirpath, dir->d_name);
        debug_print("Loading %-48s with type %-8s\n", filepath, resource_type);
        microResourceLoad(name, filepath, resource_type);
        free(name);
      }
    }
    closedir(d);
    return 1;
  }

  debug_print("Error: could not open directory %s\n", dirpath);
  return -1;
}

int microResourceLoadFont(const char *name, const char *filepath,
                          unsigned int font_size, unsigned int filter)
{
  assert(resourcesCount + 1 < MICRO_RESOURCES_MAX);

  int resourceId = microFontTTFLoad(name, filepath, font_size, filter);
  if (resourceId == -1)
    return -1;

  // store resource sorted by name in resources array
  int i = resourcesCount;
  while (i > 0 && strcompare(name, resources[i - 1].name) == -1)
  {
    resources[i] = resources[i - 1];
    i--;
  }
  resources[i].resourceId = resourceId;
  resources[i].resourceType = MICRO_TYPE_FONT;
  strcpy(resources[i].name, name);
  resourcesCount++;
  assert(resourcesCount < MICRO_RESOURCES_MAX);

  return resourceId;
}

int microResourceGet(const char *name)
{
  int key = microResourceGetKeyFromName(name);
  if (key == -1)
    return -1;
  return resources[key].resourceId;
}

int microResourceFree(const char *name)
{
  int key = microResourceGetKeyFromName(name);
  if (key == -1)
    return -1;

  Resource *res = &resources[key];
  if (res->resourceType == MICRO_TYPE_TEXTURE)
    microTextureFree(res->resourceId);
  else if (res->resourceType == MICRO_TYPE_SOUND)
    microSoundFree(res->resourceId);
  else if (res->resourceType == MICRO_TYPE_ATLAS)
    microAtlasFree(res->resourceId);
  else if (res->resourceType == MICRO_TYPE_FONT)
    microFontFree(res->resourceId);

  return 1;
}

void microResourceFreeAll()
{
  for (int i = 0; i < resourcesCount; i++)
  {
    Resource *res = &resources[i];
    switch (res->resourceType)
    {
    case MICRO_TYPE_TEXTURE:
      microTextureFree(res->resourceId);
      break;
    case MICRO_TYPE_SOUND:
      microSoundFree(res->resourceId);
      break;
    case MICRO_TYPE_ATLAS:
      microAtlasFree(res->resourceId);
      break;
    case MICRO_TYPE_FONT:
      microFontFree(res->resourceId);
      break;
    }
  }
}
