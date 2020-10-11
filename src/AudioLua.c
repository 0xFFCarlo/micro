#include "AudioLua.h"
#include "Audio.h"
#include <lua/lua.h>
#include <lua/lauxlib.h>

static int l_microLoadSoundEffect(lua_State *L)
{
    const char *filepath = luaL_checkstring(L, 1);
    int *id = lua_newuserdata(L, sizeof(int));
    *id = microLoadSoundEffect(filepath);
    luaL_getmetatable(L, "SoundEffect");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_microLoadMusic(lua_State *L)
{
    const char *filepath = luaL_checkstring(L, 1);
    int *id = lua_newuserdata(L, sizeof(int));
    *id = microLoadMusic(filepath);
    luaL_getmetatable(L, "Music");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_microPlaySound(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    int loops = luaL_checkinteger(L, 2);
    microPlaySound(*id, loops);
    return 0;
}

static int l_microIsSoundPlaying(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    int playing = microIsSoundPlaying(*id);
    lua_pushboolean(L, playing);
    return 1;
}

static int l_microStopSound(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    microStopSound(*id);
    return 0;
}

static int l_microPauseSound(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    microPauseSound(*id);
    return 0;
}

static int l_microResumeSound(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    microResumeSound(*id);
    return 0;
}

static int l_microSetSoundVolume(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    float volume = luaL_checknumber(L, 2);
    microSetSoundVolume(*id, volume);
    return 0;
}

static int l_microGetSoundVolume(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    float volume = microGetSoundVolume(*id);
    lua_pushnumber(L, volume);
    return 1;
}

static int l_microUnloadSound(lua_State *L)
{
    int *id = luaL_testudata(L, 1, "SoundEffect");
    if (id == NULL) id = luaL_checkudata(L, 1, "Music");
    microUnloadSound(*id);
    return 0;
}

void microLuaOpenAudio(lua_State *L)
{
    //register sound effect
    lua_register(L, "SoundEffect", l_microLoadSoundEffect);
    luaL_newmetatable(L, "SoundEffect");
    lua_pushcfunction(L, l_microUnloadSound);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_microPlaySound);
    lua_setfield(L, -2, "play");
    lua_pushcfunction(L, l_microStopSound);
    lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, l_microPauseSound);
    lua_setfield(L, -2, "pause");
    lua_pushcfunction(L, l_microResumeSound);
    lua_setfield(L, -2, "resume");
    lua_pushcfunction(L, l_microIsSoundPlaying);
    lua_setfield(L, -2, "isPlaying");
    lua_pushcfunction(L, l_microSetSoundVolume);
    lua_setfield(L, -2, "setVolume");
    lua_pushcfunction(L, l_microGetSoundVolume);
    lua_setfield(L, -2, "getVolume");
    lua_pop(L, 1);

    //register music
    lua_register(L, "Music", l_microLoadMusic);
    luaL_newmetatable(L, "Music");
    lua_pushcfunction(L, l_microUnloadSound);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_microPlaySound);
    lua_setfield(L, -2, "play");
    lua_pushcfunction(L, l_microStopSound);
    lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, l_microPauseSound);
    lua_setfield(L, -2, "pause");
    lua_pushcfunction(L, l_microResumeSound);
    lua_setfield(L, -2, "resume");
    lua_pushcfunction(L, l_microIsSoundPlaying);
    lua_setfield(L, -2, "isPlaying");
    lua_pushcfunction(L, l_microSetSoundVolume);
    lua_setfield(L, -2, "setVolume");
    lua_pushcfunction(L, l_microGetSoundVolume);
    lua_setfield(L, -2, "getVolume");
    lua_pop(L, 1);
}