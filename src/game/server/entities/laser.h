/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_LASER_H
#define GAME_SERVER_ENTITIES_LASER_H

#include <game/server/entity.h>

class CLaser : public CEntity
{
public:
	CLaser(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner, int Damage = -1, int MaxBounces = -1, int Delay = -1, int FakeEvalTick = -1, bool AutoDestroy = true, float DecreaseEnergyFactor = 1.0f);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

protected:
	bool HitCharacter(vec2 From, vec2 To);
	void DoBounce();

private:
	vec2 m_From;
	vec2 m_Dir;
	float m_Energy;
	int m_Damage;
	int m_Bounces;
	int m_MaxBounces;
	int m_Delay;
	int m_FakeEvalTick;
	int m_EvalTick;
	bool m_AutoDestroy;
	float m_DecreaseEnergyFactor;
	int m_Owner;
};

#endif
