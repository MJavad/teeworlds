/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
    /*DGI:Doc-Gen-Info*/
    /*DGI:Type:Server*/
    /*DGI:Exception:errorfunc*/
    /*DGI:Event:OnWeaponFire*/
    /*DGI:Event:OnJump*/
    /*DGI:Event:OnJump*/
    /*DGI:Event:OnDie*/
    /*DGI:Event:OnExplosion*/
    /*DGI:Event:OnClientEnter*/
    /*DGI:Event:OnClientConnect*/
    /*DGI:Event:OnChat*/
    /*DGI:Event:OnPlayerJoinTeam*/
    /*DGI:Event:OnNetData*/
    /*DGI:Event:OnCanSpawn*/
    /*DGI:Event:OnEntity*/
    /*DGI:Event:OnConsole*/

#include "lua.h"

#include "commands/character.cpp"
#include "commands/chat.cpp"
#include "commands/collision.cpp"
#include "commands/console.cpp"
#include "commands/config.cpp"
#include "commands/game.cpp"
#include "commands/events.cpp"
#include "commands/message.cpp"
#include "commands/player.cpp"
#include "commands/entities.cpp"
#include "commands/dummy.cpp"

#define NON_HASED_VERSION
#include <game/version.h>
#undef NON_HASED_VERSION

CLuaFile::CLuaFile()
{
    mem_zero(this, sizeof(CLuaFile));
    Close();
}

CLuaFile::~CLuaFile()
{
    #ifndef CONF_PLATFORM_MACOSX
    End();
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;
    #endif
}
void CLuaFile::Tick()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);

    FunctionPrepare("Tick");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

    lua_gc(m_pLua, LUA_GCCOLLECT, 1000);

    ErrorFunc(m_pLua);
}
void CLuaFile::TickDefered()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);

    FunctionPrepare("TickDefered");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

    ErrorFunc(m_pLua);
}
void CLuaFile::PostTick()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);

    FunctionPrepare("PostTick");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

    ErrorFunc(m_pLua);
}

void CLuaFile::End()
{
    if (m_pLua == 0)
        return;

    //try to call the end function
    //Maybe the lua file need to save data eg. a ConfigFile
    FunctionExec("end");
}

int CLuaFile::Panic(lua_State *L)
{
    dbg_break();
    return 0;
}
void CLuaFile::Init(const char *pFile)
{
    //close first
    Close();

    str_copy(m_aFilename, pFile, sizeof(m_aFilename));

    m_pLua = luaL_newstate();
    luaL_openlibs(m_pLua);

    lua_atpanic(m_pLua, &Panic);

    //include
    lua_register(m_pLua, ToLower("Include"), this->Include);
    luaL_dostring(m_pLua, "package.path = \"./lua/?.lua;./lua/lib/?.lua\"\n");

    //config
    lua_register(m_pLua, ToLower("SetScriptUseSettingPage"), this->SetScriptUseSettingPage);
    lua_register(m_pLua, ToLower("SetScriptTitle"), this->SetScriptTitle);
    lua_register(m_pLua, ToLower("SetScriptInfo"), this->SetScriptInfo);

    //events
    lua_register(m_pLua, ToLower("AddEventListener"), this->AddEventListener);
    lua_register(m_pLua, ToLower("RemoveEventListener"), this->RemoveEventListener);

    //player
    lua_register(m_pLua, ToLower("GetPlayerName"), this->GetPlayerName);
    lua_register(m_pLua, ToLower("GetPlayerClan"), this->GetPlayerClan);
    lua_register(m_pLua, ToLower("GetPlayerCountry"), this->GetPlayerCountry);
    lua_register(m_pLua, ToLower("GetPlayerScore"), this->GetPlayerScore);
    lua_register(m_pLua, ToLower("GetPlayerPing"), this->GetPlayerPing);
    lua_register(m_pLua, ToLower("GetPlayerTeam"), this->GetPlayerTeam);
    lua_register(m_pLua, ToLower("GetPlayerSkin"), this->GetPlayerSkin);
	lua_register(m_pLua, ToLower("GetPlayerColorFeet"), this->GetPlayerColorFeet);
	lua_register(m_pLua, ToLower("GetPlayerColorBody"), this->GetPlayerColorBody);
	lua_register(m_pLua, ToLower("SetPlayerScore"), this->SetPlayerScore);
	lua_register(m_pLua, ToLower("SetPlayerName"), this->SetPlayerName);
	lua_register(m_pLua, ToLower("SetPlayerTeam"), this->SetPlayerTeam);
	lua_register(m_pLua, ToLower("SetPlayerClan"), this->SetPlayerClan);
	lua_register(m_pLua, ToLower("SetPlayerCountry"), this->SetPlayerCountry);

	lua_register(m_pLua, ToLower("SetPlayerColorBody"), this->SetPlayerColorBody);
	lua_register(m_pLua, ToLower("SetPlayerColorFeet"), this->SetPlayerColorFeet);

    //character
    lua_register(m_pLua, ToLower("Emote"), this->Emote);
    lua_register(m_pLua, ToLower("GetCharacterPos"), this->GetCharacterPos);
    lua_register(m_pLua, ToLower("GetCharacterVel"), this->GetCharacterVel);
	lua_register(m_pLua, ToLower("SetCharacterPos"), this->SetCharacterPos);
    lua_register(m_pLua, ToLower("SetCharacterVel"), this->SetCharacterVel);

	//config
    lua_register(m_pLua, ToLower("GetConfigValue"), this->GetConfigValue);
    lua_register(m_pLua, ToLower("SetConfigValue"), this->SetConfigValue);

    //console
    lua_register(m_pLua, ToLower("Print"), this->Print);
    lua_register(m_pLua, ToLower("Console"), this->Console);

    //game
    lua_register(m_pLua, ToLower("GetGameType"), this->GetGameType);
    lua_register(m_pLua, ToLower("IsTeamplay"), this->IsTeamplay);

    //message
	//  lua_register(m_pLua, ToLower("GetNetError"), this->GetNetError);
	lua_register(m_pLua, ToLower("SendPacket"), this->SendPacket);
	lua_register(m_pLua, ToLower("AddModFile"), this->AddModFile);
	lua_register(m_pLua, ToLower("DeleteModFile"), this->DeleteModFile);
	lua_register(m_pLua, ToLower("SendFile"), this->SendFile);


    //collision
    lua_register(m_pLua, ToLower("IntersectLine"), this->IntersectLine);
    lua_register(m_pLua, ToLower("GetTile"), this->GetTile);
    lua_register(m_pLua, ToLower("SetTile"), this->SetTile);
    lua_register(m_pLua, ToLower("GetMapWidth"), this->GetMapWidth);
    lua_register(m_pLua, ToLower("GetMapHeight"), this->GetMapHeight);

    //Chat
    lua_register(m_pLua, ToLower("SendBroadcast"), this->SendBroadcast);
    lua_register(m_pLua, ToLower("SendChat"), this->SendChat);
    lua_register(m_pLua, ToLower("SendChatTarget"), this->SendChatTarget);

    //Entities
    lua_register(m_pLua, ToLower("EntityFind"), this->EntityFind);
    lua_register(m_pLua, ToLower("EntityGetCharacterId"), this->EntityGetCharacterId);
    lua_register(m_pLua, ToLower("EntityGetPos"), this->EntityGetPos);
    lua_register(m_pLua, ToLower("EntitySetPos"), this->EntitySetPos);
    lua_register(m_pLua, ToLower("EntityDestroy"), this->EntityDestroy);
    lua_register(m_pLua, ToLower("ProjectileFind"), this->ProjectileFind);
    lua_register(m_pLua, ToLower("ProjectileGetWeapon"), this->ProjectileGetWeapon);
    lua_register(m_pLua, ToLower("ProjectileGetOwner"), this->ProjectileGetOwner);
    lua_register(m_pLua, ToLower("ProjectileGetPos"), this->ProjectileGetPos);
    lua_register(m_pLua, ToLower("ProjectileGetDir"), this->ProjectileGetDir);
    lua_register(m_pLua, ToLower("ProjectileGetLifespan"), this->ProjectileGetLifespan);
    lua_register(m_pLua, ToLower("ProjectileGetExplosive"), this->ProjectileGetExplosive);
    lua_register(m_pLua, ToLower("ProjectileGetSoundImpact"), this->ProjectileGetSoundImpact);
    lua_register(m_pLua, ToLower("ProjectileCreate"), this->ProjectileCreate);
	lua_register(m_pLua, ToLower("LaserCreate"), this->LaserCreate);

    //game
    lua_register(m_pLua, ToLower("CreateExplosion"), this->CreateExplosion);
    lua_register(m_pLua, ToLower("CreateDeath"), this->CreateDeath);
    lua_register(m_pLua, ToLower("CreateDamageIndicator"), this->CreateDamageIndicator);

    lua_register(m_pLua, ToLower("CharacterTakeDamage"), this->CharacterTakeDamage);
    lua_register(m_pLua, ToLower("CharacterGetHealth"), this->CharacterGetHealth);
    lua_register(m_pLua, ToLower("CharacterGetArmor"), this->CharacterGetArmor);
    lua_register(m_pLua, ToLower("CharacterSetInputDirection"), this->CharacterSetInputDirection);
    lua_register(m_pLua, ToLower("CharacterSetInputJump"), this->CharacterSetInputJump);
    lua_register(m_pLua, ToLower("CharacterSetInputWeapon"), this->CharacterSetInputWeapon);
    lua_register(m_pLua, ToLower("CharacterSetInputTarget"), this->CharacterSetInputTarget);
    lua_register(m_pLua, ToLower("CharacterSetInputHook"), this->CharacterSetInputHook);
    lua_register(m_pLua, ToLower("CharacterSetInputFire"), this->CharacterSetInputFire);
    lua_register(m_pLua, ToLower("CharacterGetCoreJumped"), this->CharacterGetCoreJumped);
    lua_register(m_pLua, ToLower("CharacterSpawn"), this->CharacterSpawn);
    lua_register(m_pLua, ToLower("CharacterIsAlive"), this->CharacterIsAlive);
    lua_register(m_pLua, ToLower("CharacterKill"), this->CharacterKill);
    lua_register(m_pLua, ToLower("CharacterIsGrounded"), this->CharacterIsGrounded);
    lua_register(m_pLua, ToLower("CharacterIncreaseHealth"), this->CharacterIncreaseHealth);
    lua_register(m_pLua, ToLower("CharacterIncreaseArmor"), this->CharacterIncreaseArmor);
    lua_register(m_pLua, ToLower("CharacterSetAmmo"), this->CharacterSetAmmo);
    lua_register(m_pLua, ToLower("CharacterGetAmmo"), this->CharacterGetAmmo);
    lua_register(m_pLua, ToLower("CharacterGetInputTarget"), this->CharacterGetInputTarget);
    lua_register(m_pLua, ToLower("CharacterGetActiveWeapon"), this->CharacterGetActiveWeapon);
    lua_register(m_pLua, ToLower("CharacterDirectInput"), this->CharacterDirectInput);
    lua_register(m_pLua, ToLower("CharacterPredictedInput"), this->CharacterPredictedInput);

    lua_register(m_pLua, ToLower("SetAutoRespawn"), this->SetAutoRespawn);

    lua_register(m_pLua, ToLower("Win"), this->Win);
    lua_register(m_pLua, ToLower("SetGametype"), this->SetGametype);

	lua_register(m_pLua, ToLower("DummyCreate"), this->DummyCreate);
	lua_register(m_pLua, ToLower("IsDummy"), this->IsDummy);

    //version
    lua_register(m_pLua, ToLower("CheckVersion"), this->CheckVersion);
    lua_register(m_pLua, ToLower("GetVersion"), this->GetVersion);


    lua_pushlightuserdata(m_pLua, this);
    lua_setglobal(m_pLua, "pLUA");

    lua_register(m_pLua, ToLower("errorfunc"), this->ErrorFunc); //TODO: fix me
	lua_getglobal(m_pLua, "errorfunc");


    if (luaL_loadfile(m_pLua, m_aFilename) == 0)
    {
        lua_pcall(m_pLua, 0, LUA_MULTRET, 0);
        ErrorFunc(m_pLua);
    }
    else
    {
        ErrorFunc(m_pLua);
        dbg_msg("lua", "fail to load file: %s", pFile);
        Close();
        return;
    }
    lua_getglobal(m_pLua, "errorfunc");
    ErrorFunc(m_pLua);
}

void CLuaFile::Close()
{
    //kill lua
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;

    //clear
    mem_zero(m_aTitle, sizeof(m_aTitle));
    mem_zero(m_aInfo, sizeof(m_aInfo));
    mem_zero(m_aFilename, sizeof(m_aFilename));
    m_HaveSettings = 0;
    m_FunctionVarNum = 0;
}

int CLuaFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);

    lua_pop(L,1);

	int depth = 0;
	int frameskip = 1;
	lua_Debug frame;

    if (lua_tostring(L, -1) == 0)
        return 0;

    dbg_msg("Lua", pSelf->m_aFilename);
    dbg_msg("Lua", lua_tostring(L, -1));

    dbg_msg("Lua", "Backtrace:");
    while(lua_getstack(L, depth, &frame) == 1)
    {
        depth++;

        lua_getinfo(L, "nlSf", &frame);

        /* check for functions that just report errors. these frames just confuses more then they help */
        if(frameskip && str_comp(frame.short_src, "[C]") == 0 && frame.currentline == -1)
            continue;
        frameskip = 0;

        /* print stack frame */
        dbg_msg("Lua", "%s(%d): %s %s", frame.short_src, frame.currentline, frame.name, frame.namewhat);
    }
    lua_pop(L, 1); // remove error message
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}


void CLuaFile::ConfigClose()
{
    FunctionExec("ConfigClose");
}

void CLuaFile::PushString(const char *pString)
{
    if (m_pLua == 0)
        return;
    lua_pushstring(m_pLua, pString);
    m_FunctionVarNum++;
}

void CLuaFile::PushData(const char *pData, int Size)
{
    if (m_pLua == 0)
        return;
    lua_pushlstring(m_pLua, pData, Size);
    m_FunctionVarNum++;
}

void CLuaFile::PushInteger(int value)
{
    if (m_pLua == 0)
        return;
    lua_pushinteger(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushFloat(float value)
{
    if (m_pLua == 0)
        return;
    lua_pushnumber(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushBoolean(bool value)
{
    if (m_pLua == 0)
        return;
    lua_pushboolean(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushParameter(const char *pString)
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

bool CLuaFile::FunctionExist(const char *pFunctionName)
{
    bool Ret = false;
    if (m_pLua == 0)
        return false;
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    Ret = lua_isfunction(m_pLua, -1);
    lua_pop(m_pLua, 1);
    return Ret;
}

void CLuaFile::FunctionPrepare(const char *pFunctionName)
{
    if (m_pLua == 0 || m_aFilename[0] == 0)
        return;

    lua_getglobal(m_pLua, ToLower(pFunctionName));
    m_FunctionVarNum = 0;
}

void CLuaFile::FunctionExec(const char *pFunctionName)
{
    if (m_pLua == 0)
        return;
    if (m_aFilename[0] == 0)
        return;

    if (pFunctionName)
    {
        if (FunctionExist(pFunctionName) == false)
            return;
        FunctionPrepare(pFunctionName);
    }
    lua_pcall(m_pLua, m_FunctionVarNum, LUA_MULTRET, 0);
    ErrorFunc(m_pLua);
    m_FunctionVarNum = 0;
}


//functions

int CLuaFile::Include(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (luaL_loadfile(L, lua_tostring(L, 1)) == 0)
    {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    return 0;
}

int CLuaFile::SetScriptUseSettingPage(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    pSelf->m_HaveSettings = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::SetScriptTitle(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aTitle, lua_tostring(L, 1), sizeof(pSelf->m_aTitle));
    return 0;
}

int CLuaFile::SetScriptInfo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aInfo, lua_tostring(L, 1), sizeof(pSelf->m_aInfo));
    return 0;
}

int CLuaFile::CheckVersion(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
        lua_pushboolean(L, str_comp(GAME_LUA_VERSION, lua_tostring(L, 1)) == 0);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::GetVersion(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushstring(L, GAME_LUA_VERSION);
    return 1;
}
