#include "Audio.h"
#include "../util/debug.h"
#include <SDL2/SDL_mixer.h>

#define MICRO_MAX_SOUNDS 256
#define MICRO_MAX_MUSICS 64
#define MICRO_SOUND_TYPE_BIT 0x8000
#define MICRO_SOUND_TYPE_CLEAR_MASK 0x7FFF

static Mix_Chunk *loadedSounds[MICRO_MAX_SOUNDS];
static Mix_Music *loadedMusics[MICRO_MAX_MUSICS];
static int channelPlaying[MICRO_MAX_SOUNDS];
static u8 cleanedBuffer = SDL_FALSE;
static u8 mix_initialized = SDL_FALSE;

int microSoundLoadFromFile(const char *filepath, const int soundType)
{
  if (mix_initialized == SDL_FALSE)
  {
    assert(Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3));
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
      debug_print("Can't initialize audio: %s\n", Mix_GetError());
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
    memset(channelPlaying, -1, sizeof(u32) * MICRO_MAX_SOUNDS);
    cleanedBuffer = SDL_TRUE;
  }

  if (soundType == MICRO_SOUNDTYPE_SOUNDEFFECT)
  {
    // load sound
    Mix_Chunk *chunk = Mix_LoadWAV(filepath);
    if (chunk == NULL)
    {
      debug_print("Can't load soundeffect: %s\n", Mix_GetError());
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
      debug_print("Sound resources are full\n");
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
      debug_print("Can't load music: %s\n", Mix_GetError());
      return 0;
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
      debug_print("Music resources are full\n");
      Mix_FreeMusic(music);
      return 0;
    }
    loadedMusics[spot] = music;

    // flag sound type bit to say its a music
    spot = spot | MICRO_SOUND_TYPE_BIT;

    return spot;
  }

  return -1;
}

void microSoundPlay(const u32 soundId, const int loops)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
  {
    Mix_Music *music = loadedMusics[bufferId];
    if (music == NULL)
    {
      debug_print("Cannot play music: the soundId provided is invalid\n");
      return;
    }

    const int result = Mix_PlayMusic(music, loops == 0 ? 0 : -1);
    if (result == -1)
    {
      debug_print("Cannot play music: %s\n", Mix_GetError());
      return;
    }
  }
  else
  {
    Mix_Chunk *chunk = loadedSounds[bufferId];
    if (chunk == NULL)
    {
      debug_print("Cannot play sound: the soundId provided is invalid\n");
      return;
    }
    int channel = bufferId;
    if (channel != -1)
      channel = Mix_PlayChannel(channel, chunk, loops == 0 ? 0 : -1);
    else
      channel = Mix_PlayChannel(-1, chunk, loops == 0 ? 0 : -1);
    assert(channel != -1);

    // store in which channel the sound is playing
    channelPlaying[bufferId] = bufferId;
  }
}

void microSoundPlayNewChannel(const u32 soundId, const int loops)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
  {
    Mix_Music *music = loadedMusics[bufferId];
    if (music == NULL)
    {
      debug_print("Cannot play music: the soundId provided is invalid");
      return;
    }

    const int result = Mix_PlayMusic(music, loops == 0 ? 0 : -1);
    if (result == -1)
    {
      debug_print("Cannot play music");
      return;
    }
  }
  else
  {
    Mix_Chunk *chunk = loadedSounds[bufferId];
    if (chunk == NULL)
    {
      debug_print("Cannot play sound: the soundId provided is invalid");
      return;
    }
    int channel = Mix_PlayChannel(-1, chunk, loops == 0 ? 0 : -1);
    assert(channel != -1);

    // store in which channel the sound is playing
    channelPlaying[bufferId] = channel;
  }
}

void microSoundStop(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (channelPlaying[bufferId] == -1)
    return;

  if (isMusic)
    Mix_HaltMusic();
  else
  {
    Mix_HaltChannel(channelPlaying[bufferId]);
    channelPlaying[bufferId] = -1;
  }
}

int microSoundIsPlaying(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (channelPlaying[bufferId] == -1)
    return 0;

  if (isMusic)
    return Mix_PlayingMusic();
  else
    return Mix_Playing(channelPlaying[bufferId]);
}

void microSoundSetVolume(const u32 soundId, const float volume)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  // check for bad volume parameter
  if (volume < 0.f || volume > 1.f)
  {
    debug_print("Cannot play music");
    return;
  }
  const u32 volume_7bit = (u32)(128 * volume);

  if (isMusic)
    Mix_VolumeMusic(volume_7bit);
  else
    Mix_VolumeChunk(loadedSounds[bufferId], volume_7bit);
}

float microSoundGetVolume(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  u32 volume_7bit;
  if (isMusic)
    volume_7bit = Mix_VolumeMusic(-1);
  else
    volume_7bit = Mix_VolumeChunk(loadedSounds[bufferId], -1);

  return ((float)volume_7bit) / 128.f;
}

void microSoundPause(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    Mix_PauseMusic();
  else
    Mix_Pause(channelPlaying[bufferId]);
}

void microSoundResume(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
  if (isMusic)
    bufferId = bufferId & MICRO_SOUND_TYPE_CLEAR_MASK; // clear type bit

  if (isMusic)
    Mix_ResumeMusic();
  else
    Mix_Resume(channelPlaying[bufferId]);
}

int microSoundChannelsPlaying()
{
  return Mix_Playing(-1);
}

void microSoundFree(const u32 soundId)
{
  // check if its music or sound effects and clear sound type bit
  const bool isMusic = (soundId & MICRO_SOUND_TYPE_BIT) > 0;
  u32 bufferId = soundId;
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
