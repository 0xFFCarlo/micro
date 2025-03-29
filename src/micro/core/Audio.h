#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the audio system
 * @return 0 on success, -1 on error
 */
int microAudioInit();

/**
 * @brief Destroys the audio system
 * @return 0 on success, -1 on error
 */
int microAudioDestroy();

/**
 * @brief Loads a sound from a file and stores it in memory.
 * @param filepath The path to the file
 * @return sound id or -1 on error
 */
int microSoundLoadFromFile(const char *filepath);

/**
 * @brief Free a sound from memory
 * @param soundId The sound id
 */
void microSoundFree(const uint32_t soundId);

/**
 * @brief Plays a sound
 * @param soundId The sound id
 */
void microSoundPlay(const uint32_t soundId, const float gain);

/**
 * @brief Plays a sound at a specific position
 * @param soundId The sound id
 * @param gain The gain
 * @param loops The number of times to loop the sound
 * @param x The x position
 * @param y The y position
 */
void microSoundPlayAt(const uint32_t soundId, const float gain, const float x,
                      const float y);

/**
 * @brief Plays a sound
 * @param soundId The sound id
 * @param sourceId The source id
 */
void microSoundPlayAtSource(const uint32_t soundId, const uint32_t sourceId,
                            const char *channelTag);

/**
 * @brief Creates a new sound source
 * @return The source id
 */
uint32_t microSoundSrcNew(const char *mainChannelTag);

/**
 * @brief Creates a new channel for a sound source
 * @param sourceId The source id
 * @param channelTag The channel tag
 * @return error code
 */
uint32_t microSoundSrcNewChannel(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Free a sound source
 * @param sourceId The source id
 */
void microSoundSrcFree(const uint32_t sourceId);

/*
 * @brief Returns 1 if the sound is playing, 0 otherwise
 * @param soundId The sound id
 * @return 1 if the sound is playing, 0 otherwise
 */
int microSoundSrcIsPlaying(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Plays a sound
 * @param soundId The sound id
 * @param loops Whether to loop the sound or not
 */
void microSoundSrcLoop(const uint32_t sourceId, const char *channelTag,
                       const bool loops);

/**
 * @brief Stops a sound
 * @param soundId The sound id
 */
void microSoundSrcStop(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Stops all sounds
 */
void microSoundSrcStopAll();

/**
 * @brief Sets the position of a sound
 * @param soundId The sound id
 * @param x The x position
 * @param y The y position
 */
void microSoundSrcSetPosition(const uint32_t sourceId, float x, float y);

/**
 * @brief Spatializes a sound
 * @param soundId The sound id
 * @param spatialize Whether to spatialize the sound or not
 */
void microSoundSrcSpatialize(const uint32_t sourceId, const bool spatialize);

/**
 * @brief Pauses a sound
 * @param soundId The sound id
 */
void microSoundSrcPause(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Resumes a sound
 * @param soundId The sound id
 */
void microSoundSrcResume(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Sets the volume of a sound
 * @param soundId The sound id
 * @param volume The volume (0.0f to 1.0f)
 */
void microSoundSrcSetVolume(const uint32_t sourceId, const char *channelTag,
                            const float volume);

/**
 * @brief Gets the volume of a sound
 * @param soundId The sound id
 * @return The volume (0.0f to 1.0f)
 */
float microSoundSrcGetVolume(const uint32_t sourceId, const char *channelTag);

/**
 * @brief Set the position of the listener
 * @param x The x position
 * @param y The y position
 */
void microListenerSetPosition(float x, float y);

/**
 * @brief Set maximum distance for sound to be heard
 * @param distance The distance
 */
void microListenerSetMaxDistance(float distance);

#endif
