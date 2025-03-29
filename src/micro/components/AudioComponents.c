#include "AudioComponents.h"
#include "../core/ECS.h"
#include "../core/Audio.h"
#include <stdlib.h>
#include <assert.h>

int cid_listener = -1;
int cid_sound_source = -1;

void RegisterCListener()
{
  cid_listener = microECSComponentRegister(sizeof(CListener), NULL);
}

void CmpAddListener(int entity_id, float max_distance)
{
  assert(cid_listener != -1);
  microECSEntityAddComponent(entity_id, cid_listener,
                             &(CListener){
                               .max_distance = max_distance,
                             });
}

CListener *CmpGetListener(int entity_id)
{
  return (CListener *)microECSEntityGetComponent(entity_id, cid_listener);
}

void RegisterCSoundSource()
{
  cid_sound_source = microECSComponentRegister(sizeof(CSoundSource), NULL);
}

void CmpAddSoundSource(int entity_id, const char *mainChannelTag, float gain)
{
  assert(cid_sound_source != -1);
  const uint32_t sound_source = microSoundSrcNew(mainChannelTag);
  microSoundSrcSetVolume(sound_source, mainChannelTag, gain);
  microECSEntityAddComponent(entity_id, cid_sound_source, &(CSoundSource){
    .source_id = sound_source,
  });
}

CSoundSource *CmpGetSoundSource(int entity_id)
{
  return (CSoundSource *)microECSEntityGetComponent(entity_id, cid_sound_source);}


void RegisterAudioComponents()
{
  RegisterCListener();
  RegisterCSoundSource();
}
