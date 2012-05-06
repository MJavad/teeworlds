/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "dm.h"
#include <base/system.h>


CGameControllerDM::CGameControllerDM(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
    str_copy(m_aGameType, "MOD", sizeof(m_aGameType));
	m_pGameType = m_aGameType;
}

void CGameControllerDM::Tick()
{
	IGameController::Tick();
}
