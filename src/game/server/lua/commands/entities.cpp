/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"


int CLuaFile::EntityFind(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5))
        return 0;

    vec2 Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));

    int Max = clamp((int)lua_tointeger(L, 4), 1, 256);
    CEntity **ppEnt = new CEntity*[Max];

    int Num = pSelf->m_pServer->m_World.FindEntities(Pos, lua_tonumber(L, 3), ppEnt, Max, lua_tointeger(L, 5));
    lua_newtable(L);
    for (int i = 0; i < Num; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, ppEnt[i]->GetID());
        lua_settable(L, -3);
    }
    delete []ppEnt;
    return 1;
}

int CLuaFile::EntityGetCharacterId(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CCharacter *pChr = (CCharacter *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pChr)
    {
        lua_pushinteger(L, pChr->GetPlayer()->GetCID());
        return 1;
    }
    return 0;
}

int CLuaFile::EntityGetPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CEntity *pEnt = pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pEnt)
    {
        lua_pushnumber(L, pEnt->m_Pos.x);
        lua_pushnumber(L, pEnt->m_Pos.y);
        return 2;
    }

    return 0;
}

int CLuaFile::EntitySetPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
        return 0;

    CEntity *pEnt = pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pEnt)
    {
        pEnt->m_Pos.x = lua_tonumber(L, 2);
        pEnt->m_Pos.y = lua_tonumber(L, 3);
    }

    return 0;
}

int CLuaFile::EntityDestroy(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CEntity *pEnt = pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pEnt)
    {
        pEnt->Destroy();
    }

    return 0;
}

int CLuaFile::ProjectileFind(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    vec2 Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));

    int Max = clamp((int)lua_tointeger(L, 4), 1, 256);
    int Num = 0;

    for (CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.FindFirst(0); pPrj; pPrj = (CProjectile *)pPrj->TypeNext())
    {
		if(distance(pPrj->GetPos((pSelf->m_pServer->Server()->Tick()-pPrj->GetStartTick())/(float)pSelf->m_pServer->Server()->TickSpeed()), Pos) < lua_tonumber(L, 3)+pPrj->m_ProximityRadius)
		{
            lua_pushinteger(L, pPrj->GetID());
			Num++;
			if(Num == Max)
				break;
		}
    }
    return Num;
}

int CLuaFile::ProjectileGetWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushinteger(L, pPrj->GetWeapon());
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileGetOwner(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushinteger(L, pPrj->GetOwner());
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileGetPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        int Tick = pSelf->m_pServer->Server()->Tick()-pPrj->GetStartTick();
        if (lua_isnumber(L, 2))
            Tick += lua_tointeger(L, 2);
        vec2 Pos = pPrj->GetPos((float)Tick/(float)pSelf->m_pServer->Server()->TickSpeed());
        lua_pushnumber(L, Pos.x);
        lua_pushnumber(L, Pos.y);
        return 2;
    }
    return 0;
}

int CLuaFile::ProjectileGetDir(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        vec2 Dir = pPrj->GetDir();
        lua_pushnumber(L, Dir.x);
        lua_pushnumber(L, Dir.y);
        return 2;
    }
    return 0;
}

int CLuaFile::ProjectileGetLifespan(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushinteger(L, pPrj->GetLifespan());
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileGetExplosive(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushboolean(L, pPrj->GetExplosive());
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileGetSoundImpact(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushinteger(L, pPrj->GetSoundImpact());
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileGetStartTick(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        lua_pushinteger(L, pPrj->GetStartTick());
        return 1;
    }
    return 0;
}


int CLuaFile::ProjectileSetWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) ||! lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetWeapon(lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 0;
    }
    return 0;
}

int CLuaFile::ProjectileSetOwner(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetOwner(lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetDir(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetDir(vec2(lua_tonumber(L, 2), lua_tonumber(L, 3)));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetStartPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetPos(vec2(lua_tonumber(L, 2), lua_tonumber(L, 3)));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetLifespan(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetLifespan(lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetExplosive(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetExplosive(lua_toboolean(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetSoundImpact(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetSoundImpact(lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileSetStartTick(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    CProjectile *pPrj = (CProjectile *)pSelf->m_pServer->m_World.GetEntityByID(lua_tointeger(L, 1));
    if (pPrj)
    {
        pPrj->SetStartTick(lua_tointeger(L, 2));
        lua_pushboolean(L, true);
        return 1;
    }
    return 0;
}

int CLuaFile::ProjectileCreate(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;

    int ID = -1;
    vec2 Dir = vec2(0, -1);
    int Lifespan = 50;
    int Type = WEAPON_GUN;
    int Damage = 1;
    float Force = 0;
    bool Explosive = false;
    int ImpactSound = -1;


    if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
        Dir = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    if (lua_isnumber(L, 5))
        ID = lua_tointeger(L, 5);
    if (lua_isnumber(L, 6))
        Lifespan = lua_tointeger(L, 6);
    if (lua_isnumber(L, 7))
        Type = lua_tointeger(L, 7);
    if (lua_isnumber(L, 8))
        Damage = lua_tointeger(L, 8);
    if (lua_isnumber(L, 9))
        Force = lua_tonumber(L, 9);
    if (lua_isboolean(L, 10))
        Explosive = lua_toboolean(L, 10);
    if (lua_isnumber(L, 11))
        ImpactSound = lua_tonumber(L, 11);

    int Weapon = Type;
    if (lua_isnumber(L, 7))
        Weapon = lua_tointeger(L, 7);

    CProjectile *pProj = new CProjectile(&pSelf->m_pServer->m_World, Type,
        ID,
        vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)),
        Dir,
        Lifespan,
        Damage, Explosive, Force, ImpactSound, Weapon);

    CNetObj_Projectile p;
    pProj->FillInfo(&p);
    return 0;
}

//LaserCreate(Pos.x, Pos.y, Dir.x, Dir.y, StartEnergy, Owner)
int CLuaFile::LaserCreate(lua_State *L)
{
	lua_getglobal(L, "pLUA");
	CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
	lua_Debug Frame;
	lua_getstack(L, 1, &Frame);
	lua_getinfo(L, "nlSf", &Frame);


	vec2 Pos;
	vec2 Dir;
	float StartEnergy = pSelf->m_pServer->Tuning()->m_LaserReach;
	int Owner;
	int Damage = -1;
	int MaxBounces = -1;
	int Delay = -1;
	int FakeEvalTick = -1;
	bool AutoDestroy = true;
	float DecreaseEnergyFactor = 1.0f;

	if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5))
		return 0;

	Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
	Dir = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
	Owner = lua_tointeger(L, 5);
	if (lua_tointeger(L, 6))
        StartEnergy = lua_tonumber(L, 6);
	if (lua_isnumber(L, 7))
        Damage = lua_tointeger(L, 7);
	if (lua_isnumber(L, 8))
        MaxBounces = lua_tointeger(L, 8);
	if (lua_isnumber(L, 9))
        Delay = lua_tointeger(L, 9);
	if (lua_isnumber(L, 10))
        FakeEvalTick = lua_tointeger(L, 10);
	if (lua_isboolean(L, 11))
        AutoDestroy = lua_toboolean(L, 11);
	if (lua_isboolean(L, 12))
        DecreaseEnergyFactor = lua_tonumber(L, 12);

	CLaser *pTmp = new CLaser(&pSelf->m_pServer->m_World, Pos, Dir, StartEnergy, Owner, Damage, MaxBounces, Delay, FakeEvalTick, AutoDestroy, DecreaseEnergyFactor);
    lua_pushinteger(L, pTmp->GetID());
	return 1;
}
