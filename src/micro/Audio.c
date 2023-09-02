#include "Audio.h"
#include "../util/debug.h"
#include "Error.h"
#include <SDL2/SDL_mixer.h>

#define MICRO_MAX_SOUNDS 256
#define MICRO_MAX_MUSICS 64
#define MICRO_SOUND_TYPE_BIT 0x8000
#define MICRO_SOUND_TYPE_CLEAR_MASK 0x7FFF

static Mix_Chunk *loadedSounds[MICRO_MAX_SOUNDS];
static Mix_Music *loadedMusics[MICRO_MAX_MUSICS];
static uint8_t cleanedBuffer = SDL_FALSE;
static uint8_t mix_initialized = SDL_FALSE;

int microSoundLoadFromFile(const char *filepath, const int soundType)
{
  if (mix_initialized == SDL_FALSE)
  {
    assert(Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3));
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
      microSendError(MICRO_ERROR_FAIL, "Can't initialize audio");
      debug_print("Mix_GetError: %s\n", Mix_GetError());
      return -1;
    }
    mix_initialized = SDL_TRUE;
  }

  // initialize
  if (cleanedBuffer == SDL_FALSE)
  {
    Mix_AllocateChannels(MICRO_MAX_SOUNDS);
    memset(loadedSounds, 0, sizeof(Mix_Chunk *) * MICRO_MAX_SOUNDS);
    memset(loadedMusics, 0, sizeof(Mix_Music *) * MICRO_MAX_MUSICS);
    cleanedBuffer = SDL_TRUE;
  }

  if (soundType == MICRO_SOUNDTYPE_SOUNDEFFECT)
  {
    // load sound
    Mix_Chunk *chunk = Mix_LoadWAV(filepath);
    if (chunk == NULL)
    {
      microSendError(MICRO_ERROR_FAIL, "Can't load soundeffect");
      debug_print("Mix_GetError: %s\n", Mix_GetError());
      return -1;
    }

    // find spot to place sound in resources buffer
    int spot = -1;
    for (int i = 0; i < MICRO_MAX_SOUNDS; i++)
    {
      if (loadedSounds[i] == NULL)
      {
        spot = i;
        break;
      }
    }
    if (spot == -1)
    {
      microSendError(MICRO_ERROR_FAIL, "Sound resources are full");
      Mix_FreeChunk(chunk);
      return -1;
    }
    loadedSounds[spot] = chunk;

    return spot;
  }
  else if (soundType == MICRO_SOUNDTYPE_MUSIC)
  {
    // load music
    Mix_Music *music = Mix_LoadMUS(filepath);
    if (music == NULL)
    {
      microSendError(MICRO_ERROR_FAIL, "Can't load music");
      return MICRO_FALSE;
    }

    // find spot to place sound in resources buffer
    int spot = -1;
    for (int i = 0; i < MICRO_MAX_MUSICS; i++)
    {
      if (loadedMusics[i] == NULL)
      {
        spot = i;
        break;
      }
    }
    if (spot == -1)
    {
      microSendError(MICRO_ERROR_FAIL, "Sound resources are full");
      Mix_FreeMusic(music);
      return MICRO_FALSE;
    }
    loadedMusics[spot] = music;

    // flag sound type bit to say its a music
    spot = spot | MICRO_SOUND_TYPE_BIT;

    return spot;
  }

  return -1;
}

void microSoundPlay(const unsigned int soundId, const int loops)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
  {
    Mix_Music *music = loadedMusics[bufferId];
    if (music == NULL)
    {
      microSendError(MICRO_ERROR_FAIL,
                     "Cannot play music: the soundId provided is invalid");
      return;
    }

    const int result = Mix_PlayMusic(music, loops);
    if (result == -1)
    {
      microSendError(MICRO_ERROR_FAIL, "Cannot play music");
      return;
    }
  }
  else
  {
    Mix_Chunk *chunk = loadedSounds[bufferId];
    if (chunk == NULL)
    {
      microSendError(MICRO_ERROR_FAIL,
                     "Cannot play sound: the soundId provided is invalid");
      return;
    }
    Mix_PlayChannel(bufferId, chunk, loops);
  }
}

void microSoundStop(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    Mix_HaltMusic();
  else
    Mix_HaltChannel(bufferId);
}

int microSoundIsPlaying(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    return Mix_PlayingMusic();
  else
    return Mix_Playing(bufferId);
}

void microSoundSetVolume(const unsigned int soundId, const float volume)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  // check for bad volume parameter
  if (volume < 0.f || volume > 1.f)
  {
    microSendError(MICRO_ERROR_FAIL, "Cannot play music");
    return;
  }
  const unsigned int volume_7bit = (unsigned int)(128 * volume);

  if (isMusic)
    Mix_VolumeMusic(volume_7bit);
  else
    Mix_VolumeChunk(loadedSounds[bufferId], volume_7bit);
}

float microSoundGetVolume(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  unsigned int volume_7bit;
  if (isMusic)
    volume_7bit = Mix_VolumeMusic(-1);
  else
    volume_7bit = Mix_VolumeChunk(loadedSounds[bufferId], -1);

  return ((float)volume_7bit) / 128.f;
}

void microSoundPause(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    Mix_PauseMusic();
  else
    Mix_Pause(bufferId);
}

void microSoundResume(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    Mix_ResumeMusic();
  else
    Mix_Resume(bufferId);
}

void microSoundFree(const unsigned int soundId)
{
  // check if its music or sound effects and clear sound type bit
  const unsigned char isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  unsigned int bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
  {
    Mix_FreeMusic(loadedMusics[bufferId]);
    loadedMusics[bufferId] = NULL;
  }
  else
  {
    Mix_FreeChunk(loadedSounds[bufferId]);
    loadedSounds[bufferId] = NULL;
  }
}
