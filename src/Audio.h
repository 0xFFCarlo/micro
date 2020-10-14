#ifndef AUDIO_H
#define AUDIO_H

#define MICRO_SOUNDTYPE_SOUNDEFFECT 0
#define MICRO_SOUNDTYPE_MUSIC 1

extern int microSoundLoadFromFile(const char* filepath, const  int soundType);
extern void microSoundPlay(const unsigned int soundId, const int loops);
extern int microSoundIsPlaying(const unsigned int soundId);
extern void microSoundStop(const unsigned int soundId);
extern void microSoundPause(const unsigned int soundId);
extern void microSoundResume(const unsigned int soundId);
extern void microSoundSetVolume(const unsigned int soundId, const float volume);
extern float microSoundGetVolume(const unsigned int soundId);
extern void microSoundFree(const unsigned int soundId);

#endif
