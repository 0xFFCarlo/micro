#ifndef AUDIO_H
#define AUDIO_H

#define MICRO_SOUNDTYPE_SOUNDEFFECT 0
#define MICRO_SOUNDTYPE_MUSIC 1

/**
 * @brief Loads a sound from a file and stores it in memory.
 * @param filepath The path to the file
 * @param soundType The type of the sound (MICRO_SOUNDTYPE_SOUNDEFFECT or MICRO_SOUNDTYPE_MUSIC)
 * @return sound id or -1 on error
 */
extern int microSoundLoadFromFile(const char* filepath, const  int soundType);

/**
 * @brief Plays a sound
 * @param soundId The sound id
 * @param loops Whether to loop the sound or not
 */
extern void microSoundPlay(const unsigned int soundId, const int loops);

/*
 * @brief Returns 1 if the sound is playing, 0 otherwise
 * @param soundId The sound id
 * @return 1 if the sound is playing, 0 otherwise
 */
extern int microSoundIsPlaying(const unsigned int soundId);

/**
 * @brief Stops a sound
 * @param soundId The sound id
 */
extern void microSoundStop(const unsigned int soundId);

/**
 * @brief Pauses a sound
 * @param soundId The sound id
 */
extern void microSoundPause(const unsigned int soundId);

/**
 * @brief Resumes a sound
 * @param soundId The sound id
 */
extern void microSoundResume(const unsigned int soundId);

/**
 * @brief Sets the volume of a sound
 * @param soundId The sound id
 * @param volume The volume (0.0f to 1.0f)
 */
extern void microSoundSetVolume(const unsigned int soundId, const float volume);

/**
 * @brief Gets the volume of a sound
 * @param soundId The sound id
 * @return The volume (0.0f to 1.0f)
 */
extern float microSoundGetVolume(const unsigned int soundId);

/**
 * @brief Free a sound from memory
 * @param soundId The sound id
 */
extern void microSoundFree(const unsigned int soundId);

#endif
