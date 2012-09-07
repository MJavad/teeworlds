/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;
	m_pTele = 0;
	m_pSpeedup = 0;
}

CCollision::~CCollision()
{
}

void CCollision::Init(class CLayers *pLayers)
{
	// reset race specific pointers
	m_pTele = 0;
	m_pSpeedup = 0;

	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));
	if(m_pLayers->TeleLayer())
		m_pTele = static_cast<CTeleTile *>(m_pLayers->Map()->GetData(m_pLayers->TeleLayer()->m_Tele));
	if(m_pLayers->SpeedupLayer())
		m_pSpeedup = static_cast<CSpeedupTile *>(m_pLayers->Map()->GetData(m_pLayers->SpeedupLayer()->m_Speedup));

	for(int i = 0; i < m_Width*m_Height; i++)
	{
		//int Index = m_pTiles[i].m_Index;

        //lua eventlistener
        //OnCollisionInit
        //OnCollisionInitGetIndex
        //OnCollisionInitSetIndex
	}
}

int CCollision::GetTile(int x, int y)
{
    int Index = GetTileRaw(x, y);
    Index = Index > 128 ? 0 : Index;
    switch(Index)
    {
    case TILE_DEATH:
        Index = COLFLAG_DEATH;
        break;
    case TILE_SOLID:
        Index = COLFLAG_SOLID;
        break;
    case TILE_NOHOOK:
        Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
        break;
    default:
        Index = 0;
    }
    return Index;
	//return GetTileRaw(x, y) > 128 ? 0 : GetTileRaw(x, y);
}

int CCollision::GetTileRaw(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

    if (!m_pTiles)
        return 0;
	return m_pTiles[Ny*m_Width+Nx].m_Index;
}

void CCollision::SetTile(int x, int y, int index)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

    if (!m_pTiles)
        return;
	m_pTiles[Ny*m_Width+Nx].m_Index = index;
}

bool CCollision::IsTileSolid(int x, int y)
{
	return GetTile(x, y)&COLFLAG_SOLID;
}

// race
int CCollision::GetIndex(vec2 Pos)
{
	int nx = clamp((int)Pos.x/32, 0, m_Width-1);
	int ny = clamp((int)Pos.y/32, 0, m_Height-1);

	return ny*m_Width+nx;
}

int CCollision::GetIndex(vec2 PrevPos, vec2 Pos)
{
	float Distance = distance(PrevPos, Pos);

	if(!Distance)
	{
		int Nx = clamp((int)Pos.x/32, 0, m_Width-1);
		int Ny = clamp((int)Pos.y/32, 0, m_Height-1);

		if((m_pTiles[Ny*m_Width+Nx].m_Index >= TILE_STOPL && m_pTiles[Ny*m_Width+Nx].m_Index <= 59) ||
			(m_pTele && m_pTele[Ny*m_Width+Nx].m_Type == TILE_TELEIN) ||
			(m_pSpeedup && m_pSpeedup[Ny*m_Width+Nx].m_Force > 0))
		{
			return Ny*m_Width+Nx;
		}
	}

	float a = 0.0f;
	vec2 Tmp = vec2(0, 0);
	int Nx = 0;
	int Ny = 0;

	for(float f = 0; f < Distance; f++)
	{
		a = f/Distance;
		Tmp = mix(PrevPos, Pos, a);
		Nx = clamp((int)Tmp.x/32, 0, m_Width-1);
		Ny = clamp((int)Tmp.y/32, 0, m_Height-1);
		if((m_pTiles[Ny*m_Width+Nx].m_Index >= TILE_STOPL && m_pTiles[Ny*m_Width+Nx].m_Index <= 59) ||
			(m_pTele && m_pTele[Ny*m_Width+Nx].m_Type == TILE_TELEIN) ||
			(m_pSpeedup && m_pSpeedup[Ny*m_Width+Nx].m_Force > 0))
		{
			return Ny*m_Width+Nx;
		}
	}

	return -1;
}

vec2 CCollision::GetPos(int Index)
{
	int x = Index%m_Width;
	int y = Index/m_Width;

	return vec2(x*32+16, y*32+16);
}

int CCollision::GetTileIndex(int Index)
{
	if(Index < 0)
		return 0;

	return m_pTiles[Index].m_Index;
}

int CCollision::IsTeleport(int Index)
{
	if(!m_pTele || Index < 0)
		return 0;

	int Tele = 0;
	if(m_pTele[Index].m_Type == TILE_TELEIN)
		Tele = m_pTele[Index].m_Number;

	return Tele;
}

int CCollision::IsSpeedup(int Index)
{
	if(!m_pSpeedup || Index < 0)
		return -1;

	if(m_pSpeedup[Index].m_Force > 0)
		return Index;

	return -1;
}

void CCollision::GetSpeedup(int Index, vec2 *Dir, int *Force)
{
	float Angle = m_pSpeedup[Index].m_Angle * (3.14159265f/180.0f);
	*Force = m_pSpeedup[Index].m_Force;
	*Dir = vec2(cos(Angle), sin(Angle));
}


// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	vec2 Last = Pos0;

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
void CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces)
{
	if(pBounces)
		*pBounces = 0;

	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
		}

		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
}

bool CCollision::TestBox(vec2 Pos, vec2 Size)
{
	Size *= 0.5f;
	if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	float Distance = length(Vel);
	int Max = (int)Distance;

	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;

			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice

			if(TestBox(vec2(NewPos.x, NewPos.y), Size))
			{
				int Hits = 0;

				if(TestBox(vec2(Pos.x, NewPos.y), Size))
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					Hits++;
				}

				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					Hits++;
				}

				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}

			Pos = NewPos;
		}
	}

	*pInoutPos = Pos;
	*pInoutVel = Vel;
}
