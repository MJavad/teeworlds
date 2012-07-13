#ifndef GAME_LUAMAP_H
#define GAME_LUAMAP_H

#include <game/luaevent.h>
#include <game/mapitems.h>
#include <base/tl/array.h>

extern "C" { // lua
    #define LUA_CORE /* make sure that we don't try to import these functions */
    #include <engine/external/lua/lua.h>
    #include <engine/external/lua/lualib.h> /* luaL_openlibs */
    #include <engine/external/lua/lauxlib.h> /* luaL_loadfile */
}

class CLuaMapFile
{
    CTile *m_pTiles;
    int m_Width;
    int m_Height;
public:
    CLuaMapFile(CTile *pTiles, const char *pCode, int Width, int Height);
    ~CLuaMapFile();
    lua_State *m_pLua;
    void Tick();



    static int ErrorFunc(lua_State *L);
    static int GetTile(lua_State *L);
    static int SetTile(lua_State *L);
    static int GetFlag(lua_State *L);
    static int SetFlag(lua_State *L);
};

class CLuaMap
{
public:
    CLuaMap();
    ~CLuaMap();
    void Tick();

    array<CLuaMapFile *> m_lLuaMapFiles;

    class CLuaEventListener<CLuaMapFile> *m_pEventListener;
};

#endif
