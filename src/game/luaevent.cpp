#include "luaevent.h"
#include <base/system.h>
//storage class for the event variables

CEventVariable::CEventVariable()
{
    m_Type = EVENT_TYPE_INVALID;
    m_pData = 0;
    m_Size = 0;
}

CEventVariable::~CEventVariable()
{
    if (m_pData)
        delete [](char *)m_pData;
}

void CEventVariable::Allocate(int Size)
{
    if (m_pData)
        delete [](char *)m_pData;
    m_Size = Size;
    m_pData = 0;
    if (Size > 0)
        m_pData = new char[Size];
}

//allocate 16 bytes even if we need 4 or just 1
//why?
//because when we transform this var to something bigger
//bool -> int


void CEventVariable::Set(int Value)
{
    m_Type = EVENT_TYPE_INTEGER;
    Allocate(16);
    mem_copy(m_pData, &Value, sizeof(Value));
}

void CEventVariable::Set(long int Value)
{
    m_Type = EVENT_TYPE_INTEGER;
    Allocate(16);
    mem_copy(m_pData, &Value, sizeof(Value));
}

void CEventVariable::Set(lua_Number Value)
{
    if (Value == (int)Value)
        Set((int)Value);
    Set((float)Value);
}

void CEventVariable::Set(float Value)
{
    m_Type = EVENT_TYPE_FLOAT;
    Allocate(16);
    mem_copy(m_pData, &Value, sizeof(Value));
}

void CEventVariable::Set(bool Value)
{
    m_Type = EVENT_TYPE_BOOL;
    Allocate(16);
    mem_copy(m_pData, &Value, sizeof(Value));
}

void CEventVariable::Set(char Value)
{
    m_Type = EVENT_TYPE_CHAR;
    Allocate(16);
    mem_copy(m_pData, &Value, sizeof(Value));
}

void CEventVariable::Set(const char *pStr)
{
    m_Type = EVENT_TYPE_STRING;
    Allocate(str_length(pStr) + 1);
    mem_copy(m_pData, pStr, str_length(pStr) + 1);
}

void CEventVariable::Set(const char *pData, int Size)
{
    m_Type = EVENT_TYPE_DATA;
    Allocate(Size);
    mem_copy(m_pData, pData, Size);
}

void *CEventVariable::Get()
{
    return m_pData;
}

int CEventVariable::GetSize()
{
    return m_Size;
}

char *CEventVariable::GetString()
{
    return (char *)m_pData;
}

int CEventVariable::GetInteger()
{
    if (m_Type == EVENT_TYPE_STRING)
        return str_toint((char *)m_pData);
    if (IsNumeric() == false)
        return 0;
	if (m_Type == EVENT_TYPE_FLOAT)
		return GetFloat();
    if (!m_pData)
        return 0;
    return *((int *)m_pData);
}

float CEventVariable::GetFloat()
{
    if (m_Type == EVENT_TYPE_STRING)
        return str_tofloat((char *)m_pData);
    if (IsNumeric() == false)
        return 0;
	if (m_Type != EVENT_TYPE_FLOAT)
		return (float)GetInteger();
    if (!m_pData)
        return 0;
    return *((float *)m_pData);
}

bool CEventVariable::GetBool()
{
    if (IsNumeric() == false)
        return 0;
    if (!m_pData)
        return 0;
    return *((bool *)m_pData);
}

char CEventVariable::GetChar()
{
    if (IsNumeric() == false)
        return 0;
    if (!m_pData)
        return 0;
    return *((char *)m_pData);
}

int CEventVariable::GetType()
{
    return m_Type;
}

bool CEventVariable::IsNumeric()
{
    return !(!(m_Type & (EVENT_TYPE_INTEGER | EVENT_TYPE_FLOAT | EVENT_TYPE_BOOL | EVENT_TYPE_CHAR)));
}

void CEventVariable::Reset()
{
    Allocate(0); //free
    m_Type = EVENT_TYPE_INVALID;
}

void CEvent::Reset()
{
    for (int i = 0; i < MAX_EVENT_VARIABLES; i++)
    {
        m_aVars[i].Reset();
    }
}

CEventVariable *CEvent::FindFree()
{
    static CEventVariable Workaround; //remove me!!
    Workaround.Reset();
    for (int i = 0; i < MAX_EVENT_VARIABLES; i++)
    {
        if (m_aVars[i].GetType() == CEventVariable::EVENT_TYPE_INVALID)
        {
            return &m_aVars[i];
        }
    }
    return &Workaround;
}


//Eventlistener -> .h
