#include "luamap.h"
#include <base/math.h>
/*DGI:Doc-Gen-Info*/
/*DGI:Type:Map*/
/*DGI:Exception:errorfunc*/


#define LUA_FUNCTION_HEADER     lua_getglobal(L, "pLUA"); \
    CLuaMapFile *pSelf = (CLuaMapFile *)lua_touserdata(L, -1); \
    lua_Debug Frame; \
    lua_getstack(L, 1, &Frame); \
    lua_getinfo(L, "nlSf", &Frame); \
    ((void)(pSelf));

int CLuaMapFile::StrIsInteger(const char *pStr)
{
	while(*pStr)
	{
		if(!(*pStr >= '0' && *pStr <= '9'))
			return 0;
		pStr++;
	}
	return 1;
}

int CLuaMapFile::StrIsFloat(const char *pStr)
{
	bool Dot = false;
	while(*pStr)
	{
		if(*pStr < '0' || *pStr > '9')
		{
			if(!Dot && *pStr == '.')
				Dot = true;
			else
				return 0;
		}
		pStr++;
	}
	return 1;
}

void CLuaMapFile::PushString(const char *pString)
{
    if (m_pLua == 0)
        return;
    lua_pushstring(m_pLua, pString);
    m_FunctionVarNum++;
}

void CLuaMapFile::PushData(const char *pData, int Size)
{
    if (m_pLua == 0)
        return;
    lua_pushlstring(m_pLua, pData, Size);
    m_FunctionVarNum++;
}

void CLuaMapFile::PushInteger(int value)
{
    if (m_pLua == 0)
        return;
    lua_pushinteger(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaMapFile::PushFloat(float value)
{
    if (m_pLua == 0)
        return;
    lua_pushnumber(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaMapFile::PushBoolean(bool value)
{
    if (m_pLua == 0)
        return;
    lua_pushboolean(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaMapFile::PushParameter(const char *pString)
{
    if (m_pLua == 0)
        return;
    if (StrIsInteger(pString))
    {
        PushInteger(str_toint(pString));
    }
    else if (StrIsFloat(pString))
    {
        PushInteger(str_tofloat(pString));
    }
    else
    {
        PushString(pString);
    }

}

bool CLuaMapFile::FunctionExist(const char *pFunctionName)
{
    bool Ret = false;
    if (m_pLua == 0)
        return false;
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    Ret = lua_isfunction(m_pLua, -1);
    lua_pop(m_pLua, 1);
    return Ret;
}

void CLuaMapFile::FunctionPrepare(const char *pFunctionName)
{
    if (m_pLua == 0)
        return;

    //lua_pushstring (m_pLua, pFunctionName);
    //lua_gettable (m_pLua, LUA_GLOBALSINDEX);
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    m_FunctionVarNum = 0;
}

int CLuaMapFile::FunctionExec(const char *pFunctionName)
{
    if (m_pLua == 0)
        return 0;

    if (pFunctionName)
    {
        if (FunctionExist(pFunctionName) == false)
            return 0;
        FunctionPrepare(pFunctionName);
    }
    int Ret = lua_pcall(m_pLua, m_FunctionVarNum, LUA_MULTRET, 0);
    if (Ret)
        ErrorFunc(m_pLua);
    m_FunctionVarNum = 0;
    return Ret;
}

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
    lua_register(m_pLua, ToLower("GetWidth"), this->GetWidth);
    lua_register(m_pLua, ToLower("GetHeight"), this->GetHeight);


    luaL_dostring(m_pLua, pCode);
    ErrorFunc(m_pLua);
}

CLuaMapFile::~CLuaMapFile()
{
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;
}

void CLuaMapFile::Tick(int ServerTick)
{
    FunctionPrepare("Tick");
    PushInteger((int)(time_get() * 1000 / time_freq())); //time in ms
    PushInteger(ServerTick);
    FunctionExec();
}

int CLuaMapFile::GetTile(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if (lua_tointeger(L, 1) < 0)
            return 0;
        if (lua_tointeger(L, 1) >= pSelf->m_Width)
            return 0;
        if (lua_tointeger(L, 2) < 0)
            return 0;
        if (lua_tointeger(L, 2) >= pSelf->m_Height)
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
        if (lua_tointeger(L, 1) >= pSelf->m_Width)
            return 0;
        if (lua_tointeger(L, 2) < 0)
            return 0;
        if (lua_tointeger(L, 2) >= pSelf->m_Height)
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

int CLuaMapFile::GetWidth(lua_State *L)
{
    LUA_FUNCTION_HEADER
    lua_pushinteger(L, pSelf->m_Width);
    return 1;
}

int CLuaMapFile::GetHeight(lua_State *L)
{
    LUA_FUNCTION_HEADER
    lua_pushinteger(L, pSelf->m_Height);
    return 1;
}

int CLuaMapFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaMapFile *pSelf = (CLuaMapFile *)lua_touserdata(L, -1);
    ((void)(pSelf));

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
    Clear();
}

void CLuaMap::Tick(int ServerTick, CTile *pTiles)
{
    for (int i = 0; i < m_lLuaMapFiles.size(); i++)
    {
        if (m_lLuaMapFiles[i]->m_pTiles == pTiles || pTiles == 0)
            m_lLuaMapFiles[i]->Tick(ServerTick);
    }
}

void CLuaMap::Clear()
{
    for (int i = 0; i < m_lLuaMapFiles.size(); i++)
    {
        delete m_lLuaMapFiles[i];
    }
    m_lLuaMapFiles.clear();
}
