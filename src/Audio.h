#ifndef AUDIO_H
#define AUDIO_H

extern int microLoadSoundEffect(const char* filepath);
extern int microLoadMusic(const char* filepath);
extern void microPlaySound(const unsigned int soundId, const int loops);
extern int microIsSoundPlaying(const unsigned int soundId);
extern void microStopSound(const unsigned int soundId);
extern void microPauseSound(const unsigned int soundId);
extern void microResumeSound(const unsigned int soundId);
extern void microSetSoundVolume(const unsigned int soundId, const float volume);
extern float microGetSoundVolume(const unsigned int soundId);
extern void microUnloadSound(const unsigned int soundId);

#endif