/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"

int CLuaFile::AddEventListener(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);


    if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
        return 0;
    pSelf->m_pLuaHandler->m_pEventListener->AddEventListener(pSelf, (char *)lua_tostring(L, 1), (char *)lua_tostring(L, 2));
    return 0;
}
int CLuaFile::RemoveEventListener(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);


    if (!lua_isstring(L, 1))
        return 0;
    pSelf->m_pLuaHandler->m_pEventListener->RemoveEventListener(pSelf, (char *)lua_tostring(L, 1));
    return 0;
}
