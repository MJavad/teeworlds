#include "luarender.h"

CLuaRender::CLuaRender(int Level)
{
	m_Level = Level;
	str_format(m_aEventString, sizeof(m_aEventString), "OnRenderLevel%i", Level);
}

void CLuaRender::OnRender()
{
    m_pClient->m_pLua->m_pEventListener->CreateEventStack();
    m_pClient->m_pLua->m_pEventListener->OnEvent(m_aEventString);
}
