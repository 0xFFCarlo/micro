#include "GraphicsLua.h"
#include "Graphics.h"
#include <lua/lua.h>
#include <lua/lauxlib.h>

//texture
static int l_microLoadTextureFromFile(lua_State* L)
{
    const char *filepath = luaL_checkstring(L, 1);
    int *id = lua_newuserdata(L, sizeof(int));
    *id = microLoadSoundEffect(filepath);
    luaL_getmetatable(L, "Texture");
    lua_setmetatable(L, -2);
    return 1;
}

static int l_microGetTextureSize(lua_State* L)
{
    int *id = luaL_testudata(L, 1, "Texture");
    int width, height;
    microGetTextureSize(*id, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int l_microUnloadTexture(lua_State* L)
{
    int *id = luaL_testudata(L, 1, "Texture");
    microUnloadTexture(*id);
    return 0;
}

//view
int l_microSetView(lua_State* L)
{

}

int l_microUpdateView(lua_State* L)
{

}

int l_microSetViewViewport(lua_State* L)
{
}

int l_microSetViewCenter(lua_State* L)
{
}

int l_microSetViewSize(lua_State* L)
{
}

int l_microSetViewRotation(float rotation)
{
}

int l_microGetViewCenter(float *centerX, float *centerY)
{
}

int l_microGetViewSize(float *width, float *height)
{
}

int l_microGetViewRotation()
{
}

void microLuaOpenGraphics(lua_State* L)
{
    //register texture
    lua_register(L, "Texture", l_microLoadTextureFromFile);
    luaL_newmetatable(L, "Texture");
    lua_pushcfunction(L, l_microUnloadTexture);
    lua_setfield(L, -2, "__gc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_microGetTextureSize);
    lua_setfield(L, -2, "getSize");
    lua_pop(L, 1);

    //TODO
}