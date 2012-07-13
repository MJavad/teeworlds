#include "luamap.h"
#include <base/math.h>


#define LUA_FUNCTION_HEADER     lua_getglobal(L, "pLUA"); \
    CLuaMapFile *pSelf = (CLuaMapFile *)lua_touserdata(L, -1); \
    lua_Debug Frame; \
    lua_getstack(L, 1, &Frame); \
    lua_getinfo(L, "nlSf", &Frame); \
    (void *)pSelf;

CLuaMapFile::CLuaMapFile(CTile *pTiles, const char *pCode, int Width, int Height)
{
    m_pTiles = pTiles;
    m_pLua = luaL_newstate();
    luaL_openlibs(m_pLua);

    m_Width = Width;
    m_Height = Height;

    //lua_atpanic(m_pLua, &Panic); //Todo

    lua_pushlightuserdata(m_pLua, this);
    lua_setglobal(m_pLua, "pLUA");

    lua_register(m_pLua, ToLower("errorfunc"), this->ErrorFunc);
    lua_register(m_pLua, ToLower("GetTile"), this->GetTile);
    lua_register(m_pLua, ToLower("SetTile"), this->SetTile);
    lua_register(m_pLua, ToLower("GetFlag"), this->GetFlag);
    lua_register(m_pLua, ToLower("SetFlag"), this->SetFlag);


    luaL_dostring(m_pLua, pCode);
    ErrorFunc(m_pLua);
}

CLuaMapFile::~CLuaMapFile()
{
    lua_close(m_pLua);
}

void CLuaMapFile::Tick()
{

}

int CLuaMapFile::GetTile(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if (lua_tointeger(L, 1) < 0)
            return 0;
        if (lua_tointeger(L, 1) > pSelf->m_Width)
            return 0;
        if (lua_tointeger(L, 2) < 0)
            return 0;
        if (lua_tointeger(L, 2) > pSelf->m_Height)
            return 0;
        lua_pushinteger(L, pSelf->m_pTiles[lua_tointeger(L, 2) * pSelf->m_Width + lua_tointeger(L, 1)].m_Index);
        return 1;
    }
    return 0;
}

int CLuaMapFile::SetTile(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        if (lua_tointeger(L, 1) < 0)
            return 0;
        if (lua_tointeger(L, 1) > pSelf->m_Width)
            return 0;
        if (lua_tointeger(L, 2) < 0)
            return 0;
        if (lua_tointeger(L, 2) > pSelf->m_Height)
            return 0;
        pSelf->m_pTiles[lua_tointeger(L, 2) * pSelf->m_Width + lua_tointeger(L, 1)].m_Index = clamp((int)lua_tointeger(L, 3), 0, 255);
        return 0;
    }
    return 0;
}

int CLuaMapFile::GetFlag(lua_State *L)
{
    LUA_FUNCTION_HEADER
    return 0;
}

int CLuaMapFile::SetFlag(lua_State *L)
{
    LUA_FUNCTION_HEADER
    return 0;
}

int CLuaMapFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaMapFile *pSelf = (CLuaMapFile *)lua_touserdata(L, -1);

    lua_pop(L,1);

	int depth = 0;
	int frameskip = 1;
	lua_Debug frame;

    if (lua_tostring(L, -1) == 0)
        return 0;

    dbg_msg("LuaMap", lua_tostring(L, -1));

    dbg_msg("LuaMap", "Backtrace:");
    while(lua_getstack(L, depth, &frame) == 1)
    {
        depth++;

        lua_getinfo(L, "nlSf", &frame);

        /* check for functions that just report errors. these frames just confuses more then they help */
        if(frameskip && str_comp(frame.short_src, "[C]") == 0 && frame.currentline == -1)
            continue;
        frameskip = 0;

        /* print stack frame */
        dbg_msg("LuaMap", "%s(%d): %s %s", frame.short_src, frame.currentline, frame.name, frame.namewhat);
    }
    lua_pop(L, 1); // remove error message
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}



CLuaMap::CLuaMap()
{
    m_pEventListener = new CLuaEventListener<CLuaMapFile>();
}

CLuaMap::~CLuaMap()
{
    for (int i = 0; i < m_lLuaMapFiles.size(); i++)
    {
        delete m_lLuaMapFiles[i];
    }
}

void CLuaMap::Tick()
{
    for (int i = 0; i < m_lLuaMapFiles.size(); i++)
    {
        m_lLuaMapFiles[i]->Tick();
    }
}
