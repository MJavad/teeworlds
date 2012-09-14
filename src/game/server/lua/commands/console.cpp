/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"
#include <game/luaglobal.h>

int CLuaFile::Console(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_tointeger(L, 1))
    {
        pSelf->m_pServer->Console()->Print(lua_tointeger(L, 1), lua_tostring(L, 2), lua_tostring(L, 3));
    }
    return 0;
}

int CLuaFile::Print(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if (lua_isstring(L, 1) && lua_isstring(L, 2))
        dbg_msg(lua_tostring(L, 1), lua_tostring(L, 2));
    return 0;
}
