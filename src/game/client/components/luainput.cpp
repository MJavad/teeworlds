#include "luainput.h"

CLuaInput::CLuaInput(int Level)
{
	m_Level = Level;
	str_format(m_aEventString, sizeof(m_aEventString), "OnKeyLevel%i", Level);
}

bool CLuaInput::OnInput(IInput::CEvent Event)
{
    int EventID = m_pClient->m_pLua->m_pEventListener->CreateEventStack();
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Key);
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Unicode);
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Flags);
    m_pClient->m_pLua->m_pEventListener->OnEvent(m_aEventString);

    if (m_pClient->m_pLua->m_pEventListener->GetReturns(EventID)->m_aVars[0].IsNumeric() && m_pClient->m_pLua->m_pEventListener->GetReturns(EventID)->m_aVars[0].GetInteger() == 1)
        return true;
    return false;
}

