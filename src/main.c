#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "AudioLua.h"
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
    const int testSound = microLoadSoundEffect("res/test.wav");
    printf("Loaded sound effect\n");

    printf("TEST 1\n");
    printf("Playing sound effect fully...\n");
    microPlaySound(testSound, 0);
    while (microIsSoundPlaying(testSound)) SDL_Delay(20);
    
    printf("TEST 2\n");
    printf("Playing sound effect looping...\n");
    microPlaySound(testSound, -1);
    SDL_Delay(6000);
    
    printf("TEST 3\n");
    printf("Playing sound effect and stopping it...\n");
    microPlaySound(testSound, 0);
    SDL_Delay(700);
    microStopSound(testSound);
    printf("Sound stopped \n");
    SDL_Delay(2000);
    
    printf("TEST 4\n");
    printf("Playing sound effect pausing for 1 sec and resuming...\n");
    microPlaySound(testSound, 0);
    SDL_Delay(700);
    microPauseSound(testSound);
    printf("Sound paused \n");
    SDL_Delay(1000);
    microResumeSound(testSound);
    printf("Sound resumed \n");
    SDL_Delay(2000);
    
    printf("TEST 5\n");
    printf("Playing sound and change voume\n");
    microPlaySound(testSound, 0);
    SDL_Delay(500);
    microSetSoundVolume(testSound, 0.5);
    printf("Volume halved \n");
    SDL_Delay(500);
    microSetSoundVolume(testSound, 1.0);
    printf("Volume restored \n");
    SDL_Delay(2000);

    printf("FREE\n");
    printf("Unloading sound effect...\n");
    microUnloadSound(testSound);
    printf("Unloaded sound effect\n");
    SDL_Delay(1000);
    
    printf("SETUP\n");
    printf("Loading music...\n");
    const int testMusic = microLoadMusic("res/music.mp3");
    printf("Loaded music\n");
    
    printf("Playing music...\n");
    microPlaySound(testMusic, 0);
    SDL_Delay(1000);
    printf("Is music playing? -> %d\n", microIsSoundPlaying(testMusic));
    SDL_Delay(1000);
    microSetSoundVolume(testMusic, 0.5);
    printf("Volume halved \n");
    SDL_Delay(2000);
    microSetSoundVolume(testMusic, 1.0);
    printf("Volume restored \n");
    SDL_Delay(1000);
    microPauseSound(testMusic);
    printf("Music paused \n");
    SDL_Delay(2000);
    microResumeSound(testMusic);
    printf("Music resumed \n");
    SDL_Delay(2000);
    microStopSound(testMusic);
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

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    //create window and opengl context
    SDL_Window *window = SDL_CreateWindow("Test Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
    
    //intialize micro graphics
    microGraphicsInit();

    const unsigned int texid = microLoadTextureFromFile("res/heart.png");
    int texWidth, texHeight;
    microGetTextureSize(texid, &texWidth, &texHeight);
    printf("texture ID: %d\n", texid);
    printf("width: %d\n", texWidth);
    printf("height: %d\n", texHeight);

    if (!window)
    {
        printf("Failed to create the window\n");
        return -1;
    }


    microCreateView(0, 0, 800, 600, 400, 300, 800, 600, 0);
    microUpdateView();
    float rotation = 0.0;

    while (1)
    {
        // Get the next event
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return 0;
        }

        microClear();

        microDrawRect(texid, 0, 0, texWidth, texHeight, 32, 32, 200, 200, 0xFFFFFFFF);
        
        microDrawRectWithRotation(texid, 0, 0, texWidth, texHeight, 320, 320, 200, 200, 100, 100, rotation, 0xFFFFFFFF);
        rotation += 0.016 * 30;

        microDisplay();

        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    microGraphicsQuit();
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}

int test_lua_audio()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Failed to initialize th SDL2 library\n");
        return -1;
    }

    //Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
        return -1;

    lua_State* L;

    // initialize Lua interpreter
    L = luaL_newstate();

    // load Lua base libraries (print / math / etc)
    luaL_openlibs(L);

    //register audio module
    microLuaOpenAudio(L);

	//run lua file
	if (luaL_dofile(L, "test_audio.lua") != 0) {
        printf("Error: %s\n", lua_tostring(L, -1));
        return -1;
    }


    // Cleanup:  Deallocate all space assocatated with the lua state */
    lua_close(L);

    // Hack to prevent program from ending immediately
    printf( "Press enter to exit..." );
    getchar();
    
    // quit SDL_mixer
	Mix_CloseAudio();
    SDL_Quit();

    return 0;
}

int main(int argc, char const *argv[])
{
    //test_audio();
    //test_graphics();
    test_lua_audio();
}
