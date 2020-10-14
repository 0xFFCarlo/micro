#include "Resource.h"
#include "Audio.h"
#include "Graphics.h"

#define MICRO_RESOURCES_MAX 512
#define MICRO_RESOURCES_NAME_MAX_LEN 32

#define MICRO_TYPE_TEXTURE 0
#define MICRO_TYPE_SOUND 1
#define MICRO_TYPE_MUSIC 2


typedef struct Resource {
	char name[MICRO_RESOURCES_NAME_MAX_LEN];
	int resourceType;
	int resourceId;
} Resource;

static Resource resources[MICRO_RESOURCES_MAX];
static int resourcesCount = 0;

int strcompare(const char* stra, const char* strb)
{
	int i = 0;
	while (i < MICRO_RESOURCES_NAME_MAX_LEN) {
		if (stra[i] == '\0' || strb[i] == '\0'){
			if (strb[i] == stra[i]) return 0
			else if (stra[i] == '\0') return -1
			else return 1;
		}
		if (stra[i] > strb[i]) return 1;
		else if (stra[i] < strb[i]) return -1;
		i++;
	}
}

int microResourceGetIDFromName(const char* name)
{
	//TODO
	//binary search
	return 0;
}

int microResourceLoad(const char* name, const char* filepath, const char* type)
{
	int resourceId = -1;
	int resourceType;
	if (type == "texture")
	{
		resourceId = microTextureLoadFromFile(filepath);
		resourceType = 0;
	}
	else if (type == "sound")
	{
		resourceId = microSoundLoadFromFile(filepath, MICRO_SOUNDTYPE_SOUNDEFFECT);	
		resourceType = 1;
	}
	else if (type == "music")
	{
		resourceId = microSoundLoadFromFile(filepath, MICRO_SOUNDTYPE_MUSIC);	
		resourceType = 2;
	}

	//store resource
	Resource* res = resources[resourcesCount];
	res->resourceId = resourceId;
	res->resourceType = resourceType;
	strcpy(res->name, name);

	//TODO
	//sort by name so we can do binary search O(log(n)) access

	return resourceId;
}

int microResourceGet(const char* name)
{
	int id = microResourceGetIDFromName(name);
	if (id == -1) return -1;
	return resources[id]->resourceId;
}

int microResourceFree(const char* name)
{
	int id = microResourceGetIDFromName(name);
	if (id == -1) return -1;
	
}

int microResourceFreeAll()
{

}
