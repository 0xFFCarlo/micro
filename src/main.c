#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

#include "Audio.h"
#include "Graphics.h"

int test_audio()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Failed to initialize th SDL2 library\n");
    return -1;
  }

  //Initialize SDL_mixer
  if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    return -1;

  printf("SETUP\n");
  printf("Loading sound effect...\n");
  const int testSound = microSoundLoadFromFile("res/test.wav", MICRO_SOUNDTYPE_SOUNDEFFECT);
  printf("Loaded sound effect\n");

  printf("TEST 1\n");
  printf("Playing sound effect fully...\n");
  microSoundPlay(testSound, 0);
  while (microSoundIsPlaying(testSound)) SDL_Delay(20);

  printf("TEST 2\n");
  printf("Playing sound effect looping...\n");
  microSoundPlay(testSound, -1);
  SDL_Delay(6000);

  printf("TEST 3\n");
  printf("Playing sound effect and stopping it...\n");
  microSoundPlay(testSound, 0);
  SDL_Delay(700);
  microSoundStop(testSound);
  printf("Sound stopped \n");
  SDL_Delay(2000);

  printf("TEST 4\n");
  printf("Playing sound effect pausing for 1 sec and resuming...\n");
  microSoundPlay(testSound, 0);
  SDL_Delay(700);
  microSoundPause(testSound);
  printf("Sound paused \n");
  SDL_Delay(1000);
  microSoundResume(testSound);
  printf("Sound resumed \n");
  SDL_Delay(2000);

  printf("TEST 5\n");
  printf("Playing sound and change voume\n");
  microSoundPlay(testSound, 0);
  SDL_Delay(500);
  microSoundSetVolume(testSound, 0.5);
  printf("Volume halved \n");
  SDL_Delay(500);
  microSoundSetVolume(testSound, 1.0);
  printf("Volume restored \n");
  SDL_Delay(2000);

  printf("FREE\n");
  printf("Unloading sound effect...\n");
  microSoundFree(testSound);
  printf("Unloaded sound effect\n");
  SDL_Delay(1000);

  printf("SETUP\n");
  printf("Loading music...\n");
  const int testMusic = microSoundLoadFromFile("res/music.mp3", MICRO_SOUNDTYPE_MUSIC);
  printf("Loaded music\n");

  printf("Playing music...\n");
  microSoundPlay(testMusic, 0);
  SDL_Delay(1000);
  printf("Is music playing? -> %d\n", microSoundIsPlaying(testMusic));
  SDL_Delay(1000);
  microSoundSetVolume(testMusic, 0.5);
  printf("Volume halved \n");
  SDL_Delay(2000);
  microSoundSetVolume(testMusic, 1.0);
  printf("Volume restored \n");
  SDL_Delay(1000);
  microSoundPause(testMusic);
  printf("Music paused \n");
  SDL_Delay(2000);
  microSoundResume(testMusic);
  printf("Music resumed \n");
  SDL_Delay(2000);
  microSoundStop(testMusic);
  printf("Music stopped \n");
  SDL_Delay(2000);

  // quit SDL_mixer
  Mix_CloseAudio();
  SDL_Quit();

  return 0;
}

int test_graphics()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize th SDL2 library\n");
    return -1;
  }

  //create window and opengl context
  SDL_Window *window = SDL_CreateWindow("Test Window",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      800, 600,
      SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
      );
  
  if (!window)
  {
    printf("Failed to create the window\n");
    return -1;
  }

  //get window size
  int windowWidth, windowHeight;
  SDL_GetWindowSize(window, &windowWidth, &windowHeight);

  //intialize micro graphics
  printf("Initializing graphics...\n");
  microGraphicsInit(window);
  printf("Done\n");
  
  printf("Loading texture...\n");
  const unsigned int texid = microTextureLoadFromFile("res/heart.png");
  int texWidth, texHeight;
  microTextureGetSize(texid, &texWidth, &texHeight);
  printf("texture ID: %d\n", texid);
  printf("texture width: %d\n", texWidth);
  printf("texture height: %d\n", texHeight);

  microViewSet(0, 0, windowWidth, windowHeight,
      windowWidth/2.0, windowHeight/2.0,
      windowWidth, windowHeight, 0);
  microViewUpdate();
  float rotation = 0.0;

  while (1)
  {
    // Get the next event
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        return 0;

      if (event.type == SDL_KEYDOWN)
      {
        if (event.key.keysym.scancode == SDL_SCANCODE_Q)
          return 0;
      }
    }

    microGraphicsClear();

    microGraphicsDrawRect(texid, 0, 0, texWidth, texHeight, 32, 32, 200, 200, 1.0, 1.0, 1.0, 1.0);

    microGraphicsDrawRectRot(texid, 0, 0, texWidth, texHeight, 320, 320, 200, 200, 100, 100, rotation, 1.0, 0.0, 0.0, 1.0);
    rotation += 0.016 * 30;

    microGraphicsDisplay();

    SDL_GL_SwapWindow(window);
    SDL_Delay(16);
  }

  microGraphicsQuit();
  SDL_DestroyWindow( window );
  SDL_Quit();

  return 0;
}

int main(int argc, char const *argv[])
{
  //test_audio();
  test_graphics();
}
