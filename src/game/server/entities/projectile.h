/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

class CProjectile : public CEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon);

	vec2 GetPos(float Time);
	vec2 GetDir() { return m_Direction; }
	int GetLifespan() { return m_LifeSpan; }
	bool GetExplosive() { return m_Explosive; }
	int GetSoundImpact() { return m_SoundImpact; }
	int GetStartTick() { return m_StartTick; }
	int GetWeapon() { return m_Weapon; }
	int GetOwner() { return m_Owner; }

	void SetDir(vec2 Dir) { m_Direction = Dir; }
	void SetPos(vec2 Pos) { m_Pos = Pos; }
	void SetLifespan(int LifeSpan) { m_LifeSpan = LifeSpan; }
	void SetExplosive(bool Explosive) { m_Explosive = Explosive; }
	void SetSoundImpact(int SoundImpact) { m_SoundImpact = SoundImpact; }
	void SetStartTick(int StartTick) { m_StartTick = StartTick; }
	void SetWeapon(int Weapon) { m_Weapon = Weapon; }
	void SetOwner(int Owner) { m_Owner = Owner; }

	void FillInfo(CNetObj_Projectile *pProj);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

private:
	vec2 m_Direction;
	int m_LifeSpan;
	int m_Owner;
	int m_Type;
	int m_Damage;
	int m_SoundImpact;
	int m_Weapon;
	float m_Force;
	int m_StartTick;
	bool m_Explosive;
};

#endif
