/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"
int CLuaFile::GetGameType(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);
	if (pSelf->m_pServer->m_pController)
		lua_pushstring(L, pSelf->m_pServer->m_pController->m_pGameType);
	else
		lua_pushstring(L, "");
    return 1;
}

int CLuaFile::IsTeamplay(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (pSelf->m_pServer->m_pController)
        lua_pushboolean(L, pSelf->m_pServer->m_pController->GetGameFlags()&GAMEFLAG_TEAMS);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::CreateExplosion(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    int Owner = -1;
    int Weapon = -1;
    bool NoDamage = false;
    int Damage = 6;
    if (lua_isnumber(L, 3))
        Owner = lua_tointeger(L, 3);
    if (lua_isnumber(L, 4))
        Weapon = lua_tointeger(L, 4);
    if (lua_isboolean(L, 5))
        NoDamage = lua_toboolean(L, 5);
    if (lua_isnumber(L, 6))
        Damage = lua_tointeger(L, 6);


    pSelf->m_pServer->CreateExplosion(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), Owner, Weapon, NoDamage, Damage);

    return 0;
}

int CLuaFile::CreateDeath(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    int Who = -1;
    if (lua_isnumber(L, 3))
        Who = lua_tointeger(L, 3);

    pSelf->m_pServer->CreateDeath(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), Who);

    return 0;
}

int CLuaFile::CreateDamageIndicator(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    float Angle = 0.0f;
    int Amount = 0;
    if (lua_isnumber(L, 3))
        Angle = lua_tonumber(L, 3);
    if (lua_isnumber(L, 4))
        Amount = lua_tointeger(L, 4);

    pSelf->m_pServer->CreateDamageInd(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), Angle, Amount);

    return 0;
}

int CLuaFile::CreateHammerHit(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        pSelf->m_pServer->CreateHammerHit(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    }
    return 0;
}

int CLuaFile::CreateSound(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) &&  lua_isnumber(L, 3))
    {
        if (!lua_isnumber(L, 4))
            pSelf->m_pServer->CreateSound(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), lua_tointeger(L, 3));
        else
            pSelf->m_pServer->CreateSound(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), lua_tointeger(L, 3), lua_tonumber(L, 4));
    }
    return 0;
}

int CLuaFile::SetGametype(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
    {
        str_copy(pSelf->m_pServer->m_pController->m_aGameType, lua_tostring(L, 1), sizeof(pSelf->m_pServer->m_pController->m_aGameType));
    }
    return 0;
}

int CLuaFile::GetTuning(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
    {
        float Value = 0.0f;
        if (pSelf->m_pServer->Tuning()->Get(lua_tostring(L, 1), &Value))
        {
            lua_pushnumber(L, Value);
            return 1;
        }
        else
            return 0;
    }
    return 0;
}

int CLuaFile::SetTuning(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1) && lua_tonumber(L, 2))
    {
        float Value = 0.0f;
        pSelf->m_pServer->Tuning()->Set(lua_tostring(L, 1), lua_tonumber(L, 2));
    }
    return 0;
}

int CLuaFile::Win(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pServer->m_pController->EndRound();
    return 0;
}
