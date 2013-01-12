/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"
#include <game/luaglobal.h>

#define CEQUAL(Str) str_comp_nocase(lua_tostring(L, 1), Str) == 0

int CLuaFile::GetConfigValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

	if (!lua_isstring(L, 1))
		return 0;

	if(CEQUAL("Name"))
		lua_pushstring(L, g_Config.m_SvName);
	else if(CEQUAL("Password"))
		lua_pushstring(L, g_Config.m_Password);
	else if(CEQUAL("Map"))
		lua_pushstring(L, g_Config.m_SvMap);
	else if(CEQUAL("RconPassword"))
		lua_pushstring(L, g_Config.m_SvRconPassword);
	else if(CEQUAL("Gametype"))
		lua_pushstring(L, g_Config.m_SvGametype);
	else if(CEQUAL("MaxClients"))
		lua_pushinteger(L, g_Config.m_SvMaxClients);
	else if(CEQUAL("motd"))
		lua_pushstring(L, g_Config.m_SvMotd);
	else
		return 0;

	//be sure that a return value was pushed
	return 1;
}

int CLuaFile::SetConfigValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

	if(lua_isstring(L, 1))
	{
		if(CEQUAL("Name") && lua_isstring(L, 2))
			str_copy(g_Config.m_SvName, lua_tostring(L, 2), sizeof(g_Config.m_SvName));
		else if(CEQUAL("Password") && lua_isstring(L, 2))
			str_copy(g_Config.m_Password, lua_tostring(L, 2), sizeof(g_Config.m_Password));
		else if(CEQUAL("Map") && lua_isstring(L, 2))
			str_copy(g_Config.m_SvMap, lua_tostring(L, 2), sizeof(g_Config.m_SvMap));
		else if(CEQUAL("RconPassword") && lua_isstring(L, 2))
			str_copy(g_Config.m_SvRconPassword, lua_tostring(L, 2), sizeof(g_Config.m_SvRconPassword));
		else if(CEQUAL("Gametype") && lua_isstring(L, 2))
			str_copy(g_Config.m_SvGametype, lua_tostring(L, 2), sizeof(g_Config.m_SvGametype));
		else if(CEQUAL("MaxClients") && lua_isnumber(L, 2))
			g_Config.m_SvMaxClients = lua_tointeger(L, 2);
		else if(CEQUAL("motd") && lua_isstring(L, 2))
			str_copy(g_Config.m_SvMotd, lua_tostring(L, 2), sizeof(g_Config.m_SvMotd));
	}

	//TODO: Add more server side variables
	return 0;
}

int CLuaFile::SetAutoRespawn(lua_State *L)
{
    LUA_FUNCTION_HEADER

	if (lua_isboolean(L, 1))
        pSelf->m_pServer->m_AutoRespawn = lua_toboolean(L, 1);
	return 0;
}
