/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "particles.h"
#include "flow.h"

CParticles::CParticles()
{
	OnReset();
	m_RenderTrail.m_pParts = this;
	m_RenderExplosions.m_pParts = this;
	m_RenderGeneral.m_pParts = this;
}


void CParticles::OnReset()
{
	// reset particles
	for(int i = 0; i < MAX_PARTICLES; i++)
	{
		m_aParticles[i].m_PrevPart = i-1;
		m_aParticles[i].m_NextPart = i+1;
	}

	m_aParticles[0].m_PrevPart = 0;
	m_aParticles[MAX_PARTICLES-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CParticles::Add(int Group, CParticle *pPart)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(pInfo->m_Paused)
			return;
	}
	if (m_FirstFree == -1)
		return;

	// remove from the free list
	int Id = m_FirstFree;
	m_FirstFree = m_aParticles[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aParticles[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aParticles[Id] = *pPart;

	// insert to the group list
	m_aParticles[Id].m_PrevPart = -1;
	m_aParticles[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aParticles[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aParticles[Id].m_Life = 0;
}

void CParticles::Update(float TimePassed)
{
	static float FrictionFraction = 0;
	FrictionFraction += TimePassed;

	if(FrictionFraction > 2.0f) // safty messure
		FrictionFraction = 0;

	int FrictionCount = 0;
	while(FrictionFraction > 0.05f)
	{
		FrictionCount++;
		FrictionFraction -= 0.05f;
	}

	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aParticles[i].m_NextPart;
            // check particle death
			if(m_aParticles[i].m_Life > m_aParticles[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aParticles[i].m_PrevPart != -1)
					m_aParticles[m_aParticles[i].m_PrevPart].m_NextPart = m_aParticles[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aParticles[i].m_NextPart;

				if(m_aParticles[i].m_NextPart != -1)
					m_aParticles[m_aParticles[i].m_NextPart].m_PrevPart = m_aParticles[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aParticles[m_FirstFree].m_PrevPart = i;
				m_aParticles[i].m_PrevPart = -1;
				m_aParticles[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;

				//next and continue
				i = Next;
				continue;
			}

			m_aParticles[i].m_Life += TimePassed;
			m_aParticles[i].m_Vel += m_pClient->m_pFlow->Get(m_aParticles[i].m_Pos)*TimePassed * m_aParticles[i].m_FlowAffected;
			//m_aParticles[i].vel += flow_get(m_aParticles[i].pos)*time_passed * m_aParticles[i].flow_affected;
			m_aParticles[i].m_Vel += m_aParticles[i].m_Gravity*TimePassed;

			for(int f = 0; f < FrictionCount; f++) // apply friction
				m_aParticles[i].m_Vel *= m_aParticles[i].m_Friction;

			// move the point
			vec2 Vel = m_aParticles[i].m_Vel*TimePassed;
			Collision()->MovePoint(&m_aParticles[i].m_Pos, &Vel, 0.1f+0.9f*frandom(), NULL);
			m_aParticles[i].m_Vel = Vel / TimePassed;
			//m_aParticles[i].m_Vel = Vel* (1.0f/TimePassed); // original. slower?

			m_aParticles[i].m_Rot += TimePassed * m_aParticles[i].m_Rotspeed;

			i = Next;
		}
	}
}

void CParticles::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
        if (m_pClient->DemoPlayer()->m_Recording)
        {
            t = Client()->DemoTimeGet();
            float TimePassed = (float)((t-LastTime)/(double)time_freq());
            Update(TimePassed);
            LastTime = t;
            return;
        }
        else
        {
            const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
            if(!pInfo->m_Paused)
                Update((float)((t-LastTime)/(double)time_freq())*pInfo->m_Speed);
        }
	}
	else
		Update((float)((t-LastTime)/(double)time_freq()));

	LastTime = t;
}

void CParticles::RenderGroup(int Group)
{
    int LastTexture = -2; // std
	Graphics()->BlendNormal();
	//gfx_blend_additive();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
	Graphics()->QuadsBegin();

	int i = m_aFirstPart[Group];
	while(i != -1)
	{
		vec2 p = m_aParticles[i].m_Pos;
		float a = m_aParticles[i].m_Life / m_aParticles[i].m_LifeSpan;
		float Size = mix(m_aParticles[i].m_StartSize, m_aParticles[i].m_EndSize, a);
		if(!Graphics()->OnScreen(p.x, p.y, Size, Size)) //@matricks, oy: be lazy, just do what you must do
		{
		    i = m_aParticles[i].m_NextPart;
		    continue;
		}

        if (m_aParticles[i].m_Texture != LastTexture)
        {
            Graphics()->QuadsEnd();
            if (m_aParticles[i].m_Texture == -2)
                Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
            else
                Graphics()->TextureSet(m_aParticles[i].m_Texture);
            LastTexture = m_aParticles[i].m_Texture;
            Graphics()->QuadsBegin();
        }
        RenderTools()->SelectSprite(m_aParticles[i].m_Spr);
		vec4 Color = m_aParticles[i].m_Color;
		if (m_aParticles[i].m_ColorEnd.r >= 0 && m_aParticles[i].m_ColorEnd.g >= 0 && m_aParticles[i].m_ColorEnd.b >= 0 && m_aParticles[i].m_ColorEnd.a >= 0)
        {
            Color = mix(m_aParticles[i].m_Color, m_aParticles[i].m_ColorEnd, a);
        }

		Graphics()->QuadsSetRotation(m_aParticles[i].m_Rot);

		Graphics()->SetColor(
			Color.r,
			Color.g,
			Color.b,
			Color.a); // pow(a, 0.75f) *

		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);

		i = m_aParticles[i].m_NextPart;
	}
	Graphics()->QuadsEnd();
	Graphics()->BlendNormal();
}
