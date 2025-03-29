#ifndef AUDIO_COMPONENTS_H
#define AUDIO_COMPONENTS_H

#include <stdbool.h>

typedef struct
{
  float max_distance;
} CListener;

extern int cid_listener;
void RegisterCListener();
void CmpAddListener(int entity_id, float max_distance);
CListener *CmpGetListener(int entity_id);

typedef struct
{
  int source_id;
} CSoundSource;

extern int cid_sound_source;
void RegisterCSoundSource();
void CmpAddSoundSource(int entity_id, const char *mainChannelTag, float gain);
CSoundSource *CmpGetSoundSource(int entity_id);

void RegisterAudioComponents();

#endif // AUDIO_COMPONENTS_H
