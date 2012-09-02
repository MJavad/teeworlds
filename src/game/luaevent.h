#ifndef GAME_LUAEVENT_H
#define GAME_LUAEVENT_H

#include <base/tl/array.h>
extern "C" { // lua
    #define LUA_CORE /* make sure that we don't try to import these functions */
    #include <engine/external/lua/lua.h>
    #include <engine/external/lua/lualib.h> /* luaL_openlibs */
    #include <engine/external/lua/lauxlib.h> /* luaL_loadfile */
}
#ifdef GAME_CLIENT_LUA_H
#include "client/lua.h"
#endif
#ifdef GAME_SERVER_LUA_H
#include "server/lua/lua.h"
#endif

//storage class for the event variables

class CEventVariable
{
    int m_Type;
    void *m_pData;
    int m_Size;
    void Allocate(int Size);
public:
    CEventVariable();
    ~CEventVariable();
    void Set(int Value);
    void Set(long int Value);
    void Set(lua_Number Value);
    void Set(float Value);
    void Set(bool Value);
    void Set(char Value);
    void Set(const char *pStr);
    void Set(const char *pData, int Size);
    void *Get();
    int GetSize();
    int GetInteger();
    float GetFloat();
    bool GetBool();
    char GetChar();
    char *GetString();
    int GetType();
    bool IsNumeric();
    void Reset();

    enum
    {
        EVENT_TYPE_INVALID = 0,
        EVENT_TYPE_INTEGER = 1,
        EVENT_TYPE_FLOAT = 2,
        EVENT_TYPE_BOOL = 4,
        EVENT_TYPE_CHAR = 8,
        EVENT_TYPE_STRING = 16,
        EVENT_TYPE_DATA = 32
    };
};


#define MAX_EVENT_VARIABLES 64
class CEvent
{
public:
    CEvent()
    {
        Reset();
    }
    CEventVariable m_aVars[MAX_EVENT_VARIABLES];
    void Reset();
    CEventVariable *FindFree();
};

template <class T>
class CLuaEventListener
{
    struct CLuaListenerData
    {
        T *m_pLuaFile;
        char m_aLuaFunction[256];
        char m_aEvent[256];
        bool operator==(const CLuaListenerData &Other) { return this == &Other; }
    };

    typedef array<CLuaListenerData> TEventList;
    TEventList m_aListeners;

public:
    void AddEventListener(T *pLuaFile, char *pEvent, char *pLuaFunction);
    void RemoveEventListener(T *pLuaFile, char *pEvent);
    void RemoveAllEventListeners(T *pLuaFile);

    void OnEvent(const char *pEvent);

    #define EVENTSTACKSIZE 32
    int m_StackSize;
    CEvent m_aStackParameters[EVENTSTACKSIZE];
    CEvent m_aStackReturns[EVENTSTACKSIZE];

    int CreateEventStack();
    CEvent *GetParameters(int StackIndex);
    CEvent *GetReturns(int StackIndex);

    CLuaEventListener();
    ~CLuaEventListener();
};

//Eventlistener
template <class T>
CLuaEventListener<T>::CLuaEventListener()
{
    m_aListeners.clear();
    m_StackSize = -1;
}
template <class T>
CLuaEventListener<T>::~CLuaEventListener()
{
    m_aListeners.clear();
}

template <class T>
int CLuaEventListener<T>::CreateEventStack()
{
    return ++m_StackSize;
}

template <class T>
CEvent *CLuaEventListener<T>::GetParameters(int StackIndex)
{
    if (StackIndex < 0 || StackIndex >= EVENTSTACKSIZE)
        return 0;
    return &m_aStackParameters[StackIndex];
}

template <class T>
CEvent *CLuaEventListener<T>::GetReturns(int StackIndex)
{
    if (StackIndex < 0 || StackIndex >= EVENTSTACKSIZE)
        return 0;
    return &m_aStackReturns[StackIndex];
}

template <class T>
void CLuaEventListener<T>::AddEventListener(T *pLuaFile, char *pEvent, char *pLuaFunction)
{
    CLuaListenerData Listener;
    Listener.m_pLuaFile = pLuaFile;
    str_copy(Listener.m_aLuaFunction, pLuaFunction, sizeof(Listener.m_aLuaFunction));
    str_copy(Listener.m_aEvent, pEvent, sizeof(Listener.m_aEvent));
    m_aListeners.add(Listener);
}

template <class T>
void CLuaEventListener<T>::OnEvent(const char *pEvent)
{
    /*if (GetState())
    {
        return; //fixes a event called in an event listener
        //should check for function names! to support event by event
        //to do this we have to copy the result and the parameters -.-
        //have to be done before L1.4
    }*/
    m_aStackReturns[m_StackSize].Reset();
	for(plain_range<CLuaListenerData> r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_aEvent && str_comp_nocase(r.front().m_aEvent, pEvent) == 0)
        {
            if (r.front().m_pLuaFile->FunctionExist(r.front().m_aLuaFunction))
            {
                int Num = lua_gettop(r.front().m_pLuaFile->m_pLua); //get stack size before calling fx
                int Start = lua_gettop(r.front().m_pLuaFile->m_pLua); //get stack size before calling fx
                r.front().m_pLuaFile->FunctionPrepare(r.front().m_aLuaFunction);
                for (int i = 0; i < MAX_EVENT_VARIABLES; i++)
                {
                    if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_INTEGER)
                    {
                        r.front().m_pLuaFile->PushInteger(m_aStackParameters[m_StackSize].m_aVars[i].GetInteger());
                    }
                    else if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_CHAR)
                    {
                        r.front().m_pLuaFile->PushInteger(m_aStackParameters[m_StackSize].m_aVars[i].GetInteger());
                    }
                    else if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_FLOAT)
                    {
                        r.front().m_pLuaFile->PushFloat(m_aStackParameters[m_StackSize].m_aVars[i].GetFloat());
                    }
                    else if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_BOOL)
                    {
                        r.front().m_pLuaFile->PushBoolean(m_aStackParameters[m_StackSize].m_aVars[i].GetInteger());
                    }
                    else if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_STRING)
                    {
                        r.front().m_pLuaFile->PushString(m_aStackParameters[m_StackSize].m_aVars[i].GetString());
                    }
                    else if (m_aStackParameters[m_StackSize].m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_DATA)
                    {
                        r.front().m_pLuaFile->PushData((char *)m_aStackParameters[m_StackSize].m_aVars[i].Get(), m_aStackParameters[m_StackSize].m_aVars[i].GetSize());
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
                	if (lua_isnumber(r.front().m_pLuaFile->m_pLua, Start + i + 1))
                    {
                        if (lua_tointeger(r.front().m_pLuaFile->m_pLua, Start + i + 1) == lua_tonumber(r.front().m_pLuaFile->m_pLua, Start + i + 1))
                            m_aStackReturns[m_StackSize].m_aVars[i].Set((int)lua_tointeger(r.front().m_pLuaFile->m_pLua, Start + i + 1));
                        else
                            m_aStackReturns[m_StackSize].m_aVars[i].Set((float)lua_tonumber(r.front().m_pLuaFile->m_pLua, Start + i + 1));
                    }
                    else if (lua_isboolean(r.front().m_pLuaFile->m_pLua, Start + i + 1))
                    {
                        m_aStackReturns[m_StackSize].m_aVars[i].Set(lua_toboolean(r.front().m_pLuaFile->m_pLua, Start + i + 1));
                    }
                    else if (lua_isstring(r.front().m_pLuaFile->m_pLua, Start + i + 1))
                    {
                        int Size = 0;
                        const char *pData = lua_tolstring(r.front().m_pLuaFile->m_pLua, Start + i + 1, (size_t *)&Size);
                        if (str_length(pData) == Size)
                            m_aStackReturns[m_StackSize].m_aVars[i].Set(pData);
                        else
                            m_aStackReturns[m_StackSize].m_aVars[i].Set(pData, Size);
                    }
                }
                for (int i = 0; i < Num; i++) //pop values
                {
                    lua_remove(r.front().m_pLuaFile->m_pLua, -1); //remove value last first
                    //this solves the recusive-function-call-stack problem
                    //lua_pop(r.front().m_pLuaFile->m_pLua, 1);
                }
            }
        }
    }
    m_aStackParameters[m_StackSize].Reset(); //
    m_StackSize--;
}

template <class T>
void CLuaEventListener<T>::RemoveEventListener(T *pLuaFile, char *pEvent)
{
	for(plain_range<CLuaListenerData> r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_pLuaFile == pLuaFile && str_comp_nocase(r.front().m_aEvent, pEvent) == 0)
        {
            m_aListeners.remove(r.front());
        }
    }
}

template <class T>
void CLuaEventListener<T>::RemoveAllEventListeners(T *pLuaFile)
{
	for(plain_range<CLuaListenerData> r = m_aListeners.all(); !r.empty(); r.pop_front())
    {
        if (r.front().m_pLuaFile == pLuaFile)
        {
            m_aListeners.remove(r.front());
        }
    }
}

#endif
