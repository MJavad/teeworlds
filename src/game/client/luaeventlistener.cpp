#include "lua.h"

CLuaEventListener::CLuaEventListener()
{
    m_aListeners.clear();
}

CLuaEventListener::~CLuaEventListener()
{
    m_aListeners.clear();
}


void CLuaEventListener::AddEventListener(class CLuaFile *pLuaFile, char *pEvent, char *pLuaFunction)
{
    CLuaListenerData Listener;
    Listener.m_pLuaFile = pLuaFile;
    str_copy(Listener.m_aLuaFunction, pLuaFunction, sizeof(Listener.m_aLuaFunction));
    str_copy(Listener.m_aEvent, pEvent, sizeof(Listener.m_aEvent));
    m_aListeners.add(Listener);
}

void CLuaEventListener::OnEvent(const char *pEvent)
{
    m_Returns.Reset();
	for(array<CLuaListenerData>::range r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_aEvent && str_comp_nocase(r.front().m_aEvent, pEvent) == 0)
        {
            if (r.front().m_pLuaFile->FunctionExist(r.front().m_aLuaFunction))
            {
                int Num = lua_gettop(r.front().m_pLuaFile->m_pLua); //get stack size before calling fx
                r.front().m_pLuaFile->FunctionPrepare(r.front().m_aLuaFunction);
                for (int i = 0; i < MAX_EVENTS; i++)
                {
                    if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_INTEGER)
                    {
                        r.front().m_pLuaFile->PushInteger(m_Parameters.m_aVars[i].GetInteger());
                    }
                    else if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_CHAR)
                    {
                        r.front().m_pLuaFile->PushInteger(m_Parameters.m_aVars[i].GetInteger());
                    }
                    else if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_FLOAT)
                    {
                        r.front().m_pLuaFile->PushFloat(m_Parameters.m_aVars[i].GetInteger());
                    }
                    else if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_BOOL)
                    {
                        r.front().m_pLuaFile->PushBoolean(m_Parameters.m_aVars[i].GetInteger());
                    }
                    else if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_STRING)
                    {
                        r.front().m_pLuaFile->PushString(m_Parameters.m_aVars[i].GetString());
                    }
                    else if (m_Parameters.m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_DATA)
                    {
                        r.front().m_pLuaFile->PushData((char *)m_Parameters.m_aVars[i].Get(), m_Parameters.m_aVars[i].GetSize());
                    }
                    else
                    {
                        break;
                    }
                }
                r.front().m_pLuaFile->FunctionExec();
                Num = lua_gettop(r.front().m_pLuaFile->m_pLua) - Num; //get number of returns
                for (int i = 0; i < Num; i++) //lua stack begins with 1 - so manipulate every i
                {
                    if (lua_isboolean(r.front().m_pLuaFile->m_pLua, i + 1))
                    {
                        dbg_msg("", "%i is bool");
                        m_Returns.m_aVars[i].Set(lua_toboolean(r.front().m_pLuaFile->m_pLua, i + 1));
                    }
                    if (lua_isnumber(r.front().m_pLuaFile->m_pLua, i + 1))
                    {
                        dbg_msg("", "%i is number");
                        if (lua_tointeger(r.front().m_pLuaFile->m_pLua, i + 1) == lua_tonumber(r.front().m_pLuaFile->m_pLua, i + 1))
                            m_Returns.m_aVars[i].Set(lua_tointeger(r.front().m_pLuaFile->m_pLua, i + 1));
                        else
                            m_Returns.m_aVars[i].Set((float)lua_tonumber(r.front().m_pLuaFile->m_pLua, i + 1));
                    }
                    if (lua_isstring(r.front().m_pLuaFile->m_pLua, i + 1))
                    {
                        dbg_msg("", "%i is string");
                        int Size = 0;
                        const char *pData = lua_tolstring(r.front().m_pLuaFile->m_pLua, i + 1, (size_t *)&Size);
                        if (str_length(pData) == Size)
                            m_Returns.m_aVars[i].Set(pData);
                        else
                            m_Returns.m_aVars[i].Set(pData, Size);
                    }
                    lua_pop(r.front().m_pLuaFile->m_pLua, 1);
                }
            }
        }
    }
    m_Parameters.Reset();
}

void CLuaEventListener::RemoveEventListener(class CLuaFile *pLuaFile, char *pEvent)
{
	for(array<CLuaListenerData>::range r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_pLuaFile == pLuaFile && str_comp_nocase(r.front().m_aEvent, pEvent) == 0)
        {
            m_aListeners.remove(r.front());
        }
    }
}

void CLuaEventListener::RemoveAllEventListeners(class CLuaFile *pLuaFile)
{
	for(array<CLuaListenerData>::range r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_pLuaFile == pLuaFile)
        {
            m_aListeners.remove(r.front());
        }
    }
}
