#include "ambience_music.h"
#include "../micro/Audio.h"
#include "../micro/Resources.h"

AmbienceType ambience_type = AMBIENCE_NONE;
uint32_t ambience_music_ids[AMBIENCE_COUNT] = {0};

void ambienceMusicSetup()
{
  // Load musics resources
  uint32_t normal_id = microResourceLoad(
    "music", "./res/sounds/music_desolate_world_DSTechnician.mp3", "music");
  ambience_music_ids[AMBIENCE_NORMAL] = normal_id;
}

void ambienceMusicSet(AmbienceType type)
{
  if (ambience_type != AMBIENCE_NONE && ambience_type != type)
    microSoundStop(ambience_music_ids[ambience_type]);
  ambience_type = type;
  microSoundPlay(ambience_music_ids[ambience_type], 1);
}
