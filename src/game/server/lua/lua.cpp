/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "lua.h"
#include "string.h"
#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/sound.h>
#include <engine/console.h>
void CLua::Tick()
{
    if (m_ConsoleInit == false)
    {
        m_pServer->Console()->RegisterPrintCallback(IConsole::OUTPUT_LEVEL_DEBUG, ConsolePrintCallback, this);
        m_ConsoleInit = true;
    }
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        if (m_aLuaFiles[i].GetScriptName()[0] == 0 && m_pServer->m_pLuaCore->GetFileDir(i)[0])
            m_aLuaFiles[i].Init(m_pServer->m_pLuaCore->GetFileDir(i));
        else if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileDir(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].Tick();
    }
    for (array<char *>::range r = m_lpEvalBuffer.all(); !r.empty(); r.pop_front())
    {
        for (int i = 0; i < MAX_LUA_FILES; i++)
        {
            if (m_aLuaFiles[i].m_pLua)
                luaL_dostring(m_aLuaFiles[i].m_pLua, r.front());
        }
    }
    m_lpEvalBuffer.clear();
}
void CLua::TickDefered()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
       if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileDir(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].TickDefered();
    }
}
void CLua::PostTick()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
		if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileDir(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].PostTick();
    }
}

void CLua::Eval(const char *pCode)
{
    char *pBuf = new char[str_length(pCode) + 1];
    str_copy(pBuf, pCode, str_length(pCode) + 1);
    m_lpEvalBuffer.add(pBuf);
}

void CLua::End()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        m_aLuaFiles[i].End();
    }
}

CLua::CLua(CGameContext *pServer)
{
    Close();

    m_pServer = pServer;

    m_pEventListener = new CLuaEventListener<CLuaFile>();

    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        m_aLuaFiles[i].m_pServer = pServer;
        m_aLuaFiles[i].m_pLuaHandler = this;
    }
}

CLua::~CLua()
{
    Close();
}

void CLua::Close()
{
    End();
}

int CLua::GetFileId(char *pFileDir)
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        if (str_comp(m_aLuaFiles[i].GetScriptName(), pFileDir) == 0)
            return i;
    }
    return -1;
}

void CLua::ConfigClose(char *pFileDir)
{
    int Id = GetFileId(pFileDir);
    if (Id == -1)
        return;
    m_aLuaFiles[Id].ConfigClose();
}

void CLua::ConsolePrintCallback(const char *pLine, void *pUserData)
{
    CLua *pSelf = (CLua *)pUserData;
    int EventID = pSelf->m_pEventListener->CreateEventStack();
    pSelf->m_pEventListener->GetParameters(EventID)->FindFree()->Set((char *)pLine);
    pSelf->m_pEventListener->OnEvent("OnConsole");
}
