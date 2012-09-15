/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"


int CLuaFile::GetCharacterPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

	 if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
    {
        lua_pushnumber(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_Pos.x);
        lua_pushnumber(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_Pos.y);
        return 2;
    }
    return 0;
}
int CLuaFile::SetCharacterPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
		{
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Pos.x = lua_tonumber(L, 2);
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Pos.y = lua_tonumber(L, 3);
		}
	}
	return 0;
}

int CLuaFile::GetCharacterVel(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
    {
        lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.x);
        lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.y);
        return 2;
    }
    return 0;
}
int CLuaFile::SetCharacterVel(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
		if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
		{
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.x = lua_tonumber(L, 2);
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.y = lua_tonumber(L, 3);
			return 0;
		}
	}
	return 0;
}

int CLuaFile::Emote(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
       if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
	   {
            int StopTick = lua_isnumber(L, 3) ? lua_tointeger(L, 3) : pSelf->m_pServer->Server()->Tick() + pSelf->m_pServer->Server()->TickSpeed();
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->SetEmote(lua_tointeger(L, 2), StopTick);
			return 0;
	   }
    }
    return 0;
}

int CLuaFile::CharacterSpawn(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter() == 0 && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_DieTick < pSelf->m_pServer->Server()->Tick())
        {
            if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
                pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->Spawn(vec2(lua_tonumber(L, 2), lua_tonumber(L, 3)));
            else
                pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->TryRespawn(true);
			return 0;
        }
    }
    return 0;
}

int CLuaFile::CharacterIsAlive(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
        {
            if (pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
            {
                lua_pushboolean(L, true);
            }
            else
            {
                lua_pushboolean(L, false);
            }
            return 1;
        }
    }
    return 0;
}

int CLuaFile::CharacterKill(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->Die(lua_isnumber(L, 3) ? lua_tointeger(L, 3) : lua_tointeger(L, 1), lua_isnumber(L, 2) ? lua_tointeger(L, 2) : WEAPON_GAME);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputDirection(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_Direction = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputJump(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_Jump = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_WantedWeapon = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputTarget(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_TargetX = lua_tointeger(L, 2);
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_TargetY = lua_tointeger(L, 3);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputHook(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_Hook = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetInputFire(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_Fire = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterGetInputTarget(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_TargetX);
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput()->m_TargetY);
            return 2;
       }
    }
    return 0;
}

int CLuaFile::CharacterDirectInput(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->OnDirectInput(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput());
        }
    }
    return 0;
}

int CLuaFile::CharacterPredictedInput(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->OnPredictedInput(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetInput());
        }
    }
    return 0;
}

int CLuaFile::CharacterGetActiveWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_ActiveWeapon);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::CharacterSetActiveWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter() && lua_tointeger(L, 2))
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_ActiveWeapon = lua_tointeger(L, 2);
        }
    }
    return 0;
}

int CLuaFile::CharacterGetCoreJumped(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetCore()->m_Jumped);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::CharacterIsGrounded(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushboolean(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->IsGrounded());
            return 1;
        }
    }
    return 0;
}

int CLuaFile::CharacterIncreaseHealth(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            int Add = lua_isnumber(L, 2) ? lua_tointeger(L, 2) : 1;
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->IncreaseHealth(Add, lua_isnumber(L, 3) ? lua_tointeger(L, 3) : 10);
        }
    }
    return 0;
}

int CLuaFile::CharacterIncreaseArmor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            int Add = lua_isnumber(L, 2) ? lua_tointeger(L, 2) : 1;
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->IncreaseArmor(Add, lua_isnumber(L, 3) ? lua_tointeger(L, 3) : 10);
        }
    }
    return 0;
}

int CLuaFile::CharacterSetHealth(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter() && lua_isnumber(L, 2))
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->SetHealth(lua_tointeger(L, 2));
        }
    }
    return 0;
}

int CLuaFile::CharacterSetArmor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter() && lua_isnumber(L, 2))
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->SetArmor(lua_tointeger(L, 2));
        }
    }
    return 0;
}

int CLuaFile::CharacterGetHealth(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetHealth());
        }
    }
    return 0;
}

int CLuaFile::CharacterGetArmor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GetArmor());
        }
    }
    return 0;
}

int CLuaFile::CharacterSetAmmo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_tointeger(L, 2) >= 0 && lua_tointeger(L, 2) < NUM_WEAPONS)
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            if (lua_tointeger(L, 2) == WEAPON_NINJA)
            {
                if (lua_isnumber(L, 3) && lua_tointeger(L, 3))
                    pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->GiveNinja(lua_isnumber(L, 4) ? lua_tointeger(L, 4) : 0);
                else
                    pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->TakeNinja();

            }
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_aWeapons[lua_tointeger(L, 2)].m_Ammo = lua_isnumber(L, 3) ? lua_tointeger(L, 3) : 10;
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_aWeapons[lua_tointeger(L, 2)].m_Got = (pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_aWeapons[lua_tointeger(L, 2)].m_Ammo != 0);
        }
    }
    return 0;
}

int CLuaFile::CharacterGetAmmo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_tointeger(L, 2) >= 0 && lua_tointeger(L, 2) < NUM_WEAPONS)
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_aWeapons[lua_tointeger(L, 2)].m_Ammo);
            lua_pushinteger(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_aWeapons[lua_tointeger(L, 2)].m_Got);
            return 2;
        }
    }
    return 0;
}

int CLuaFile::CharacterTakeDamage(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            vec2 Force = vec2(0, 0);
            if (lua_isnumber(L, 5) && lua_isnumber(L, 6))
                Force = vec2(lua_tonumber(L, 5), lua_tonumber(L, 6));
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->TakeDamage(Force, lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        }
    }
    return 0;
}

int CLuaFile::SendCharacterInfo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
        {
            pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->Snap(-1);
            //or
            /*
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->Snap(i);
            }
            */
        }
    }
    return 0;
}
