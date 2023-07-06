#include "Resources.h"
#include "Audio.h"
#include "Graphics.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MICRO_RESOURCES_MAX 512
#define MICRO_RESOURCES_NAME_MAX_LEN 32
#define MICRO_TYPE_TEXTURE 0
#define MICRO_TYPE_SOUND 1
#define MICRO_TYPE_MUSIC 2
#define MICRO_TYPE_ATLAS 3


typedef struct Resource {
  char name[MICRO_RESOURCES_NAME_MAX_LEN];
  int resourceType;
  int resourceId;
} Resource;

static Resource resources[MICRO_RESOURCES_MAX];
static int resourcesCount = 0;

#define CMP_EQUAL 0
int strcompare(const char* stra, const char* strb)
{
  int i = 0;
  while (i < MICRO_RESOURCES_NAME_MAX_LEN) {
    if (stra[i] == '\0' || strb[i] == '\0'){
      if (strb[i] == stra[i]) return 0;
      else if (stra[i] == '\0') return -1;
      else return 1;
    }
    if (stra[i] > strb[i]) return 1;
    else if (stra[i] < strb[i]) return -1;
    i++;
  }
  return -1;
}

int microResourceGetIDFromName(const char* name)
{
  //Binary search
  int min = 0;
  int max = resourcesCount - 1;
  int mid = (min + max) / 2;
  while (min <= max) {
    mid = (min + max) / 2;
    int cmp = strcompare(name, resources[mid].name);
    if (cmp == CMP_EQUAL) return mid;
    else if (cmp == -1) max = mid - 1;
    else min = mid + 1;
  }
  return 0;
}

int microResourceLoad(const char* name, const char* filepath, const char* type)
{
  assert(resourcesCount + 1 < MICRO_RESOURCES_MAX);

  int resourceId = -1;
  int resourceType = -1;
  if (strcompare(type, "texture") == CMP_EQUAL)
  {
    resourceId = microTextureLoadFromFile(filepath);
    resourceType = MICRO_TYPE_TEXTURE;
  }
  else if (strcompare(type, "atlas") == CMP_EQUAL)
  {
    resourceId = microTextureAtlasLoadFromPath(filepath);
    resourceType = MICRO_TYPE_ATLAS;
  }
  else if (strcompare(type, "sound") == CMP_EQUAL)
  {
    resourceId = microSoundLoadFromFile(filepath, MICRO_SOUNDTYPE_SOUNDEFFECT);	
    resourceType = MICRO_TYPE_SOUND;
  }
  else if (strcompare(type, "music") == CMP_EQUAL)
  {
    resourceId = microSoundLoadFromFile(filepath, MICRO_SOUNDTYPE_MUSIC);	
    resourceType = MICRO_TYPE_MUSIC;
  }
  else
  {
    printf("Error: resource type %s not supported\n", type);
    return -1;
  }

  //store resource sorted by name in resources array
  int i = resourcesCount;
  while (i > 0 && strcompare(name, resources[i - 1].name) == -1) {
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

int microResourceGet(const char* name)
{
  int id = microResourceGetIDFromName(name);
  if (id == -1) return -1;
  return resources[id].resourceId;
}

int microResourceFree(const char* name)
{
  int id = microResourceGetIDFromName(name);
  if (id == -1) return -1;

  Resource* res = &resources[id];
  if (res->resourceType == MICRO_TYPE_TEXTURE)
    microTextureFree(res->resourceId);
  else if (res->resourceType == MICRO_TYPE_SOUND || res->resourceType == MICRO_TYPE_MUSIC)
    microSoundFree(res->resourceId);
  else if (res->resourceType == MICRO_TYPE_ATLAS)
    microTextureAtlasFree(res->resourceId);

  return 1;
}

void microResourceFreeAll()
{
  for (int i = 0; i < resourcesCount; i++)
  {
    Resource* res = &resources[i];
    if (res->resourceType == MICRO_TYPE_TEXTURE)
    {
      microTextureFree(res->resourceId);
    }
    else if (res->resourceType == MICRO_TYPE_SOUND || res->resourceType == MICRO_TYPE_MUSIC)
    {
      microSoundFree(res->resourceId);
    }
    else if (res->resourceType == MICRO_TYPE_ATLAS)
    {
      microTextureAtlasFree(res->resourceId);
    }
  }	
}
