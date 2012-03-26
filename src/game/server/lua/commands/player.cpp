/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"

inline float HueToRgb(float v1, float v2, float h)
{
	if(h < 0.0f) h += 1;
	if(h > 1.0f) h -= 1;
	if((6.0f * h) < 1.0f) return v1 + (v2 - v1) * 6.0f * h;
	if((2.0f * h) < 1.0f) return v2;
	if((3.0f * h) < 2.0f) return v1 + (v2 - v1) * ((2.0f/3.0f) - h) * 6.0f;
	return v1;
}


inline vec3 HslToRgb(vec3 HSL)
{
	if(HSL.s == 0.0f)
		return vec3(HSL.l, HSL.l, HSL.l);
	else
	{
		float v2 = HSL.l < 0.5f ? HSL.l * (1.0f + HSL.s) : (HSL.l+HSL.s) - (HSL.s*HSL.l);
		float v1 = 2.0f * HSL.l - v2;

		return vec3(HueToRgb(v1, v2, HSL.h + (1.0f/3.0f)), HueToRgb(v1, v2, HSL.h), HueToRgb(v1, v2, HSL.h - (1.0f/3.0f)));
	}
}

inline vec3 RgbToHsl(vec3 RGB)
{
	vec3 HSL;
	float MaxVar, MinVar;

	MinVar = min(min(RGB.r, RGB.g), RGB.b);
	MaxVar = max(max(RGB.r, RGB.g), RGB.b);
	
	HSL.l = (MaxVar+MinVar)/2.0f;

	if(MaxVar == MinVar)
	{
		HSL.s = 0;
		HSL.h = 0;
	}
	else
	{
		float d = MaxVar - MinVar;
		HSL.s = HSL.l > 0.5f ? d / (2 - MaxVar - MinVar) : d / (MaxVar + MinVar); 
		
		if(MaxVar == RGB.r)
			HSL.h = (RGB.g - RGB.b) / d + (RGB.g < RGB.b ? 6 : 0);
		else if(MaxVar == RGB.g)
			HSL.h = (RGB.b - RGB.r) / d + 2;
		else if(MaxVar == RGB.b)
			HSL.h = (RGB.r - RGB.g) / d + 4;

		HSL.h /= 6.0f;
	}
	
	if(HSL.l <= 0.5f)
		HSL.l = 0.0f;
	else
		HSL.l = (HSL.l-0.5f)/0.005f*0.01f;

	return HSL;
}

int CLuaFile::GetPlayerName(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
		if (str_comp(pSelf->m_pServer->Server()->ClientName(lua_tointeger(L, 1)) , "(invalid)") != 0 &&  str_comp(pSelf->m_pServer->Server()->ClientName(lua_tointeger(L, 1)) , "(connecting)") != 0)
			lua_pushstring(L, pSelf->m_pServer->Server()->ClientName(lua_tointeger(L, 1)));
        else
            lua_pushnil(L);      
    }
    else
        lua_pushnil(L);
    return 1;
}

int CLuaFile::GetPlayerClan(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        lua_pushstring(L, pSelf->m_pServer->Server()->ClientClan(lua_tointeger(L, 1)));
        return 1;        
    }
    return 0;
}

int CLuaFile::GetPlayerCountry(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
		lua_pushinteger(L, pSelf->m_pServer->Server()->ClientCountry(lua_tointeger(L, 1)));
        return 1;       
    }
    return 0;
}

int CLuaFile::GetPlayerScore(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
			if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
			{
				lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_Score);
				return 1;
			}
            lua_pushinteger(L, 0);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerPing(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
			{
				lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_Latency.m_Min);
				return 1;
			}    
        }
    }
    return 0;
}

int CLuaFile::GetPlayerTeam(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
			{
				lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetTeam());
				return 1;
			}
        }
    }
    return 0;
}

int CLuaFile::GetPlayerSkin(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
		
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
			{
				lua_pushstring(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_TeeInfos.m_SkinName);
				return 1;
			}
			lua_pushstring(L, "");
            return 1;
        }
    }
    return 0;
}

//SetPlayerScore(ClientID, Score)
int CLuaFile::SetPlayerScore(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	if(lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
		{
			if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
				pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_Score = lua_tointeger(L, 2);
		}
	}
	return 0;
}

//SetPlayerName(ClientID, Name)
int CLuaFile::SetPlayerName(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);


	if(lua_isnumber(L, 1) && lua_isstring(L, 2))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
		{
			pSelf->m_pServer->Server()->SetClientName(lua_tointeger(L, 1), lua_tostring(L, 2));
		}
	}

	return 0;
}

//SetPlayerClan(ClientID, Clan)
int CLuaFile::SetPlayerClan(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);
	
	if(lua_isnumber(L, 1) && lua_isstring(L, 2))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
		{
			pSelf->m_pServer->Server()->SetClientClan(lua_tointeger(L, 1), lua_tostring(L, 2));
		}
	}

	return 0;
}

//SetPlayerCountry(ClientID, CountryID)
int CLuaFile::SetPlayerCountry(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);



	if(lua_isnumber(L, 1) && lua_isnumber(L, 2))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
		{
			pSelf->m_pServer->Server()->SetClientCountry(lua_tointeger(L, 1), lua_tointeger(L, 2));
		}
	}

	return 0;
}

//GetPlayerColorFeet(ClientID)
int CLuaFile::GetPlayerColorFeet(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	if(lua_isnumber(L, 1))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
		{
			int v = pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_TeeInfos.m_ColorFeet;
			vec3 Color = HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
			lua_pushnumber(L, Color.r);
			lua_pushnumber(L, Color.g);
			lua_pushnumber(L, Color.b);
			return 3;
		}
	}

	return 0;
}

//GetPlayerColorBody(ClientID)
int CLuaFile::GetPlayerColorBody(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	if(lua_isnumber(L, 1))
	{
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
		{
			int v = pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_TeeInfos.m_ColorBody;
			vec3 Color = HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));

			lua_pushnumber(L, Color.r);
			lua_pushnumber(L, Color.g);
			lua_pushnumber(L, Color.b);
			return 3;
		}
	}

	return 0;
}

//SetPlayerColorBody(ClientID, r, g, b)
int CLuaFile::SetPlayerColorBody(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return 0;

	int ClientID = lua_tonumber(L, 1);
	
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[ClientID])
	{
		vec3 RGB = vec3(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
		vec3 HSL = RgbToHsl(RGB);

		int v = ((int)(HSL.h*255.0f)<<16) + ((int)(HSL.s*255.0f)<<8) + (int)(HSL.l*255.0f); 
		pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_TeeInfos.m_ColorBody = v;
	}

	return 0;
}

//SetPlayerColorFeet(ClientID, r, g, b)
int CLuaFile::SetPlayerColorFeet(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);

	if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return 0;

	int ClientID = lua_tonumber(L, 1);
	
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[ClientID])
	{
		vec3 RGB = vec3(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
		vec3 HSL = RgbToHsl(RGB);

		int v = ((int)(HSL.h*255.0f)<<16) + ((int)(HSL.s*255.0f)<<8) + (int)(HSL.l*255.0f); 
		pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_TeeInfos.m_ColorFeet = v;
	}

	return 0;
}

