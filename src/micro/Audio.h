#ifndef AUDIO_H
#define AUDIO_H

#include "Types.h"

#define MICRO_SOUNDTYPE_SOUNDEFFECT 0
#define MICRO_SOUNDTYPE_MUSIC 1

/**
 * @brief Loads a sound from a file and stores it in memory.
 * @param filepath The path to the file
 * @param soundType The type of the sound (MICRO_SOUNDTYPE_SOUNDEFFECT or
 * MICRO_SOUNDTYPE_MUSIC)
 * @return sound id or -1 on error
 */
extern int microSoundLoadFromFile(const char *filepath, const int soundType);

/**
 * @brief Plays a sound
 * @param soundId The sound id
 * @param loops Whether to loop the sound or not
 */
extern void microSoundPlay(const u32 soundId, const int loops);

/**
 * @brief Plays a sound in a new channel
 * @param soundId The sound id
 * @param loops Whether to loop the sound or not
 */
extern void microSoundPlayNewChannel(const u32 soundId,
                                     const int loops);

/*
 * @brief Returns 1 if the sound is playing, 0 otherwise
 * @param soundId The sound id
 * @return 1 if the sound is playing, 0 otherwise
 */
extern int microSoundIsPlaying(const u32 soundId);

/**
 * @brief Stops a sound
 * @param soundId The sound id
 */
extern void microSoundStop(const u32 soundId);

/**
 * @brief Pauses a sound
 * @param soundId The sound id
 */
extern void microSoundPause(const u32 soundId);

/**
 * @brief Resumes a sound
 * @param soundId The sound id
 */
extern void microSoundResume(const u32 soundId);

/**
 * @brief Sets the volume of a sound
 * @param soundId The sound id
 * @param volume The volume (0.0f to 1.0f)
 */
extern void microSoundSetVolume(const u32 soundId, const float volume);

/**
 * @brief Gets the volume of a sound
 * @param soundId The sound id
 * @return The volume (0.0f to 1.0f)
 */
extern float microSoundGetVolume(const u32 soundId);

/**
 * @brief Get number of channels playing a sound
 */
extern int microSoundChannelsPlaying();

/**
 * @brief Free a sound from memory
 * @param soundId The sound id
 */
extern void microSoundFree(const u32 soundId);

#endif
