#include "Audio.h"
#include "../util/debug.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <memory.h>
#include <stdio.h>
#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"
#include "minimp3/minimp3_ex.h"

#define MICRO_MAX_SOUNDS 128
#define MICRO_MAX_SOURCES_STATIC 64
#define MICRO_MAX_SOURCES_DYNAMIC 64
#define MICRO_MAX_CHANNELS 8

typedef struct MultiChannelSource
{
  ALuint sourceId[MICRO_MAX_CHANNELS];
  const char *channelTag[MICRO_MAX_CHANNELS];
  uint32_t channelCount;
} MultiChannelSource;

static ALint sounds[MICRO_MAX_SOUNDS];
static uint32_t sounds_search_index = 0;
static ALint sources_static[MICRO_MAX_SOURCES_STATIC];
static MultiChannelSource sources_dynamic[MICRO_MAX_SOURCES_DYNAMIC];
static uint32_t source_search_index = 0;
static ALCdevice *device = NULL;
static ALCcontext *context = NULL;
static mp3dec_t mp3d;
static float source_max_distance = 1000.0f;
static float source_rolloff_factor = 2.0f / 1000.0f;

// statistics
static uint32_t sound_count = 0;
// static uint32_t sounds_data_size = 0; TODO:

static void _check_al_error(const char *file, int line)
{
  ALenum error = alGetError();
  const char* errorMsg = NULL;
  switch (error)
  {
  case AL_NO_ERROR:
    return;
  case AL_INVALID_NAME:
    errorMsg = "AL_INVALID_NAME";
    break;
  case AL_INVALID_ENUM:
    errorMsg = "AL_INVALID_ENUM";
    break;
  case AL_INVALID_VALUE:
    errorMsg = "AL_INVALID_VALUE";
    break;
  case AL_INVALID_OPERATION:
    errorMsg = "AL_INVALID_OPERATION";
    break;
  case AL_OUT_OF_MEMORY:
    errorMsg = "AL_OUT_OF_MEMORY";
    break;
  default:
    errorMsg = "UNKNOWN";
    break;
  }
  if (error != AL_NO_ERROR)
    debug_print("OpenAL error: %s (%d) at %s:%d\n", errorMsg, error, file, line);
}

#define check_al_error() _check_al_error(__FILE__, __LINE__)

// Struct for WAV file header
typedef struct
{
  char chunkID[4];
  uint32_t chunkSize;
  char format[4];
  char subchunk1ID[4];
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char subchunk2ID[4];
  uint32_t subchunk2Size;
} WAVHeader;

// Function to load WAV file
static int load_wav(const char *filename, ALuint *buffer, ALenum *format,
                    ALsizei *size, ALsizei *frequency)
{
  WAVHeader header;
  FILE *file = fopen(filename, "rb");
  if (!file)
  {
    debug_print("Failed to open file\n");
    return -1;
  }

  // Read header
  fread(&header, sizeof(WAVHeader), 1, file);
  if (header.bitsPerSample == 16)
  {
    if (header.numChannels == 1)
      *format = AL_FORMAT_MONO16;
    else if (header.numChannels == 2)
      *format = AL_FORMAT_STEREO16;
  }
  else if (header.bitsPerSample == 8)
  {
    if (header.numChannels == 1)
      *format = AL_FORMAT_MONO8;
    else if (header.numChannels == 2)
      *format = AL_FORMAT_STEREO8;
  }
  else
  {
    debug_print("Unsupported WAV format\n");
    return -1;
  }

  *size = header.subchunk2Size;
  *frequency = header.sampleRate;

  unsigned char *data = (unsigned char *)malloc(header.subchunk2Size);
  if (!data)
  {
    fclose(file);
    debug_print("Memory allocation error\n");
    return -1;
  }

  // Read data
  fread(data, header.subchunk2Size, 1, file);
  fclose(file);

  // Buffer the data
  alBufferData(*buffer, *format, data, *size, *frequency);
  free(data);
  check_al_error();

  return 0;
}

static int load_mp3(const char *filename, ALuint *buffer, ALenum *format,
                    ALsizei *size, ALsizei *frequency)
{
  mp3dec_file_info_t info;
  int result;

  result = mp3dec_load(&mp3d, filename, &info, NULL, NULL);
  if (result != 0)
  {
    debug_print("Error loading MP3 file\n");
    return -1;
  }

  // Buffer audio data
  *format = info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  alBufferData(*buffer, *format, info.buffer,
               info.samples * sizeof(mp3d_sample_t), info.hz);
  *size = info.samples * sizeof(mp3d_sample_t);
  *frequency = info.hz;
  free(info.buffer);

  return 0;
}

int microAudioInit()
{
  if (device == NULL)
  {
    device = alcOpenDevice(NULL);
    if (device == NULL)
    {
      debug_print("Cannot open audio device\n");
      return -1;
    }

    context = alcCreateContext(device, NULL);
    if (context == NULL)
    {
      debug_print("Cannot create audio context\n");
      return -1;
    }

    if (alcMakeContextCurrent(context) == false)
    {
      debug_print("Could not set OpenAL context!\n");
      return -1;
    }

    memset(sounds, -1, sizeof(ALint) * MICRO_MAX_SOUNDS);
    memset(sources_static, 0, sizeof(ALint) * MICRO_MAX_SOURCES_STATIC);
    memset(sources_dynamic, 0,
           sizeof(MultiChannelSource) * MICRO_MAX_SOURCES_DYNAMIC);

    // Pre-allocate sources_static
    for (int i = 0; i < MICRO_MAX_SOURCES_STATIC; i++)
    {
      ALuint source = 0;
      alGenSources(1, &source);
      sources_static[i] = source;
    }

    mp3dec_init(&mp3d);
  }

  return 0;
}

int microAudioDestroy()
{
  if (device != NULL)
  {
    microSoundSrcStopAll();

    for (int i = 0; i < MICRO_MAX_SOURCES_STATIC; i++)
    {
      const ALuint source = sources_static[i];
      alDeleteSources(1, &source);
      check_al_error();
    }

    for (int i = 0; i < MICRO_MAX_SOURCES_DYNAMIC; i++)
    {
      for (int j = 0; j < (int)sources_dynamic[i].channelCount; j++)
      {
        const ALuint source = sources_dynamic[i].sourceId[j];
        alDeleteSources(1, &source);
        check_al_error();
      }
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
    device = NULL;
    context = NULL;
  }

  return 0;
}

static int find_unused_source()
{
  int is_playing;
  const uint32_t initial_source = source_search_index;
  do
  {
    alGetSourcei(sources_static[source_search_index], AL_SOURCE_STATE,
                 &is_playing);
    if (is_playing == AL_STOPPED || is_playing == AL_INITIAL)
      return source_search_index;
    source_search_index = (source_search_index + 1) % MICRO_MAX_SOURCES_STATIC;
  } while (source_search_index != initial_source);
  return -1;
}

static int find_unused_sound()
{
  const uint32_t initial_sound = sounds_search_index;
  do
  {
    if (sounds[sounds_search_index] == -1)
      return sounds_search_index;
    sounds_search_index = (sounds_search_index + 1) % MICRO_MAX_SOUNDS;
  } while (sounds_search_index != initial_sound);
  return -1;
}

int microSoundLoadFromFile(const char *filepath)
{
  ALuint buffer = 0;
  ALenum format;
  ALsizei size, frequency;
  int rc = -1;

  if (microAudioInit() == -1)
    goto exit;

  // load sound
  alGenBuffers(1, &buffer);

  // Load sound data based on file extension
  if (strstr(filepath, ".wav") != NULL)
  {
    if (load_wav(filepath, &buffer, &format, &size, &frequency) != 0)
    {
      debug_print("Cannot load sound effect\n");
      goto exit_buffer;
    }
  }
  else if (strstr(filepath, ".mp3") != NULL)
  {
    if (load_mp3(filepath, &buffer, &format, &size, &frequency) != 0)
    {
      debug_print("Cannot load sound effect\n");
      goto exit_buffer;
    }
  }
  else
  {
    debug_print("Unsupported file format\n");
    goto exit_buffer;
  }

  // find spot to place sound in resources buffer
  int spot = find_unused_sound();
  if (spot == -1)
  {
    debug_print("Sound resources are full\n");
    goto exit_buffer;
  }

  sounds[spot] = buffer;
  rc = spot;
  sound_count++;

exit:

  return rc;

exit_buffer:
  alDeleteBuffers(1, &buffer);
  return rc;
}

void microSoundFree(const uint32_t soundId)
{
  const ALuint buffer = (ALuint)sounds[soundId];
  if (sounds[soundId] == -1)
    return;
  alDeleteBuffers(1, &buffer);
  sounds[soundId] = -1;
  sound_count--;
  check_al_error();
}

void microSoundPlay(const uint32_t soundId, const float gain)
{
  const ALuint buffer = sounds[soundId];
  int source_id = find_unused_source();
  if (source_id == -1)
  {
    debug_print("No available sources\n");
    return;
  }

  alSourcei(sources_static[source_id], AL_BUFFER, buffer);
  alSourcef(sources_static[source_id], AL_GAIN, gain);
  // No spatialization
  alSourcei(sources_static[source_id], AL_SOURCE_RELATIVE, AL_TRUE);
  alSource3f(sources_static[source_id], AL_POSITION, 0.f, 0.f, 0.f);

  alSourcePlay(sources_static[source_id]);
}

void microSoundPlayAt(const uint32_t soundId, const float gain, const float x,
                      const float y)
{
  const ALuint buffer = sounds[soundId];
  int source_id = find_unused_source();
  if (source_id == -1)
  {
    debug_print("No available sources\n");
    return;
  }

  alSourcei(sources_static[source_id], AL_BUFFER, buffer);
  alSourcef(sources_static[source_id], AL_GAIN, gain);
  // Attenuation
  alSourcef(sources_static[source_id], AL_REFERENCE_DISTANCE, 1.0f);
  alSourcef(sources_static[source_id], AL_MAX_DISTANCE, source_max_distance);
  alSourcef(sources_static[source_id], AL_ROLLOFF_FACTOR,
            source_rolloff_factor);
  // Spatialization
  alSourcei(sources_static[source_id], AL_SOURCE_RELATIVE, AL_FALSE);
  alSource3f(sources_static[source_id], AL_POSITION, x, y, 0.f);

  alSourcePlay(sources_static[source_id]);
}

static int32_t microSoundSrcGetChannel(const uint32_t sourceId,
                                       const char *channelTag)
{
  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  for (int i = 0; i < (int)mcs->channelCount; i++)
  {
    if (strcmp(mcs->channelTag[i], channelTag) == 0)
      return mcs->sourceId[i];
  }

  return -1;
}

void microSoundPlayAtSource(const uint32_t soundId, const uint32_t sourceId,
                            const char *channelTag)
{
  int alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  const ALuint buffer = sounds[soundId];
  alSourcei(alSourceId, AL_BUFFER, buffer);
  alSourcePlay(alSourceId);
}

uint32_t microSoundSrcNewChannel(const uint32_t sourceId,
                                 const char *channelTag)
{
  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  if (mcs->channelCount == MICRO_MAX_CHANNELS)
  {
    debug_print("No available channels\n");
    return -1;
  }

  ALuint alSource = 0;
  alGenSources(1, &alSource);
  check_al_error();
  // Attenuation
  alSourcef(alSource, AL_REFERENCE_DISTANCE, 1.0f);
  alSourcef(alSource, AL_MAX_DISTANCE, source_max_distance);
  alSourcef(alSource, AL_ROLLOFF_FACTOR, source_rolloff_factor);

  mcs->sourceId[mcs->channelCount] = alSource;
  mcs->channelTag[mcs->channelCount] = channelTag;
  mcs->channelCount++;

  return sourceId;
}

uint32_t microSoundSrcNew(const char *mainChannelTag)
{
  int sourceId = -1;
  for (int i = 0; i < MICRO_MAX_SOURCES_DYNAMIC; i++)
  {
    if (sources_dynamic[i].channelCount != 0)
      continue;
    sourceId = i;
    break;
  }

  if (sourceId == -1)
  {
    debug_print("No available sources\n");
    return -1;
  }

  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  mcs->channelCount = 0;
  microSoundSrcNewChannel(sourceId, mainChannelTag);

  return sourceId;
}

uint32_t microSoundSrcFreeChannel(const uint32_t sourceId,
                                  const char *channelTag)
{
  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  for (int i = 0; i < (int)mcs->channelCount; i++)
  {
    if (strcmp(mcs->channelTag[i], channelTag) != 0)
      continue;
    alDeleteSources(1, &mcs->sourceId[i]);
    mcs->sourceId[i] = mcs->sourceId[mcs->channelCount - 1];
    mcs->channelTag[i] = mcs->channelTag[mcs->channelCount - 1];
    mcs->channelCount--;
    return sourceId;
  }

  return -1;
}

void microSoundSrcFree(const uint32_t sourceId)
{
  int dynamic_index = sourceId % MICRO_MAX_SOURCES_DYNAMIC;
  MultiChannelSource *mcs = &sources_dynamic[dynamic_index];
  for (int i = 0; i < (int)mcs->channelCount; i++)
  {
    alDeleteSources(1, &mcs->sourceId[i]);
    check_al_error();
  }
  mcs->channelCount = 0;
}

int microSoundSrcIsPlaying(const uint32_t sourceId, const char *channelTag)
{
  ALint state;
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return -1;
  alGetSourcei(alSourceId, AL_SOURCE_STATE, &state);
  check_al_error();
  return state == AL_PLAYING;
}

void microSoundSrcLoop(const uint32_t sourceId, const char *channelTag,
                       const bool loops)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  alSourcei(alSourceId, AL_LOOPING, loops);
  check_al_error();
}

void microSoundSrcStop(const uint32_t sourceId, const char *channelTag)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  alSourceStop(alSourceId);
  alSourcei(alSourceId, AL_BUFFER, 0);
  check_al_error();
}

void microSoundSrcStopAll()
{
  for (int i = 0; i < MICRO_MAX_SOURCES_STATIC; i++)
  {
    alSourceStop(sources_static[i]);
    alSourcei(sources_static[i], AL_BUFFER, 0);
  }

  for (int i = 0; i < MICRO_MAX_SOURCES_DYNAMIC; i++)
  {
    for (int j = 0; j < (int)sources_dynamic[i].channelCount; j++)
    {
      alSourceStop(sources_dynamic[i].sourceId[j]);
      alSourcei(sources_dynamic[i].sourceId[j], AL_BUFFER, 0);
    }
  }
}

void microSoundSrcSetPosition(const uint32_t sourceId, float x, float y)
{
  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  for (int i = 0; i < (int)mcs->channelCount; i++)
  {
    int alSourceId = mcs->sourceId[i];
    alSource3f(alSourceId, AL_POSITION, x, y, 0.f);
    check_al_error();
  }
}

void microSoundSrcPause(const uint32_t sourceId, const char *channelTag)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  alSourcePause(alSourceId);
  check_al_error();
}

void microSoundSrcResume(const uint32_t sourceId, const char *channelTag)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  alSourcePlay(alSourceId);
  check_al_error();
}

void microSoundSrcSetVolume(const uint32_t sourceId, const char *channelTag,
                            const float volume)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return;
  alSourcef(alSourceId, AL_GAIN, volume);
  check_al_error();
}

float microSoundSrcGetVolume(const uint32_t sourceId, const char *channelTag)
{
  int32_t alSourceId = microSoundSrcGetChannel(sourceId, channelTag);
  if (alSourceId == -1)
    return -1;
  float volume;
  alGetSourcef(alSourceId, AL_GAIN, &volume);
  check_al_error();
  return volume;
}

void microSoundSrcSpatialize(const uint32_t sourceId, const bool spatialize)
{
  MultiChannelSource *mcs = &sources_dynamic[sourceId];
  for (int i = 0; i < (int)mcs->channelCount; i++)
  {
    int alSourceId = mcs->sourceId[i];
    alSourcei(alSourceId, AL_SOURCE_RELATIVE, spatialize ? AL_FALSE : AL_TRUE);
    alSource3f(alSourceId, AL_POSITION, 0.f, 0.f, 0.f);
    check_al_error();
  }
}

void microListenerSetPosition(float x, float y)
{
  alListener3f(AL_POSITION, x, y, 0.f);
}

void microListenerSetMaxDistance(float distance)
{
  source_max_distance = distance;
  source_rolloff_factor = 2.0f / distance;
}
