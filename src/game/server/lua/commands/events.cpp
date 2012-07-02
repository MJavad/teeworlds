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
    pSelf->m_pLuaHandler->m_EventListener.AddEventListener(pSelf, (char *)lua_tostring(L, 1), (char *)lua_tostring(L, 2));
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
    pSelf->m_pLuaHandler->m_EventListener.RemoveEventListener(pSelf, (char *)lua_tostring(L, 1));
    return 0;
}
//Kill-Events
/*
int CLuaFile::KillGetKillerID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillKillerID);
    return 1;
}
int CLuaFile::KillGetVictimID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillVictimID);
    return 1;
}
int CLuaFile::KillGetWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillWeapon);
    return 1;
}*/
//Chat-Events
int CLuaFile::ChatGetText(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushstring(L, pSelf->m_pLuaHandler->m_EventListener.m_pChatText);
    return 1;
}
int CLuaFile::ChatGetTeam(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ChatTeam);
    return 1;
}
int CLuaFile::ChatHide(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_ChatHide = true;
    return 0;
}

int CLuaFile::WeaponFireGetWeaponID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireWeaponID);
    return 1;
}

int CLuaFile::WeaponFireGetDir(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireDir.x);
    lua_pushnumber(L, pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireDir.y);
    return 2;
}



int CLuaFile::WeaponFireSetReloadTime(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
        pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireReloadTimer = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::WeaponFireDisableSound(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireDisableSound = true;
    return 0;
}

int CLuaFile::WeaponFireAutoFire(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isboolean(L, 1))
        pSelf->m_pLuaHandler->m_EventListener.m_OnWeaponFireAutoFire = lua_toboolean(L, 1);
    return 0;
}


int CLuaFile::JumpGetJumpID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnJumpJumpID);
    return 1;
}

int CLuaFile::ExplosionGetDamage(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ExplosionDamage);
    return 1;
}

int CLuaFile::ExplosionGetOwner(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ExplosionOwner);
    return 1;
}

int CLuaFile::ExplosionGetWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ExplosionWeapon);
    return 1;
}

int CLuaFile::ExplosionGetPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ExplosionPos.x);
    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ExplosionPos.y);
    return 2;
}

int CLuaFile::ExplosionAbort(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_ExplosionAbort = true;
    return 0;
}

int CLuaFile::DieGetVictimID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnDieVictimID);
    return 1;
}

int CLuaFile::DieGetKillerID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnDieKillerID);
    return 1;
}

int CLuaFile::DieGetWeaponID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnDieWeaponID);
    return 1;
}
int CLuaFile::AbortSpawn(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_AbortSpawn = true;
    return 0;
}

int CLuaFile::SpawnGetTeam(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_SpawnTeam);
    return 1;
}

int CLuaFile::GetSelectedTeam(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_SelectedTeam);

	return 1;
}

int CLuaFile::AbortTeamJoin(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	pSelf->m_pLuaHandler->m_EventListener.m_AbortTeamJoin = true;

	return 0;
}/*
int CLuaFile::GetOnTilePosIndex(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnTilePosIndex);

	return 1;
}
int CLuaFile::GetOnTileIndex(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnTileIndex);

	return 1;
}
int CLuaFile::SetOnTileIndex(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);
	pSelf->m_pLuaHandler->m_EventListener.m_OnTileIndex = lua_tointeger(L, 1);
	return 0;
}*/
int CLuaFile::OnEntityGetPos(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnEntityPosition.x);
	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnEntityPosition.y);

	return 1;
}
int CLuaFile::OnEntityGetIndex(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_OnEntityIndex);

	return 1;
}
int CLuaFile::OnEntitySetIndex(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	pSelf->m_pLuaHandler->m_EventListener.m_OnEntityIndex = lua_tointeger(L, 1);
	return 0;
}

int CLuaFile::EventGetCID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_EventCID);
    return 1;
}
