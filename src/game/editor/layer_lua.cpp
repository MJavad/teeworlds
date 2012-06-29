/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "editor.h"


CLayerLua::CLayerLua()
{
	str_copy(m_aName, "Lua", sizeof(m_aName));
	mem_zero(m_aLuaCode, sizeof(m_aLuaCode));
}

CLayerLua::~CLayerLua()
{
}

int CLayerLua::RenderProperties(CUIRect *pToolbox)
{
	return 0;
}
