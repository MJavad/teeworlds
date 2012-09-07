/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#ifdef GAME_SERVER_LUA_H

#include "../lua.h"

#include <game/luaglobal.h>
void CLuaFile::MySQLTick()
{
    for (int i = 0; i < m_lpResults.GetSize(); i++)
    {
        if (m_lpResults[i]->m_Timestamp + time_freq() < time_get())
        {
            MySQLFreeResult(i, m_lpResults[i]->m_QueryId);
            i--;
        }
        else
        {
            int EventID = m_pLuaHandler->m_pEventListener->CreateEventStack();
            m_pLuaHandler->m_pEventListener->GetParameters(EventID)->FindFree()->Set(m_lpResults[i]->m_QueryId);
            m_pLuaHandler->m_pEventListener->OnEvent("OnMySQLResults");
        }
    }
}

void CLuaFile::MySQLFreeAll()
{
    for (int i = 0; i < m_lpResults.GetSize(); i++)
    {
        MySQLFreeResult(i, m_lpResults[i]->m_QueryId);
        i--;
    }
}

void CLuaFile::MySQLFreeResult(int Id, int QueryId)
{
    CResults *pResult = 0;
    if (m_lpResults[Id]->m_QueryId == QueryId) //got the right
    {
        pResult = m_lpResults[Id];
        m_lpResults.DeleteByIndex(Id);
    }
    else //find the right
    {
        for (int i = 0; i < m_lpResults.GetSize(); i++)
        {
            if (m_lpResults[i]->m_QueryId == QueryId)
            {
                pResult = m_lpResults[i];
                m_lpResults.DeleteByIndex(i);
            }
        }
    }
    if (pResult == 0)
        return;

    for (int RowId = 0; RowId < pResult->m_lpRows.GetSize(); RowId++)
    {
        CRow *pRow = pResult->m_lpRows[RowId];
        for (int FieldId = 0; FieldId < pRow->m_lpFields.GetSize(); FieldId++)
        {
            CField *pField = pRow->m_lpFields[FieldId];
            delete pField;
        }
        delete pRow;
    }
    delete pResult;

}

void CLuaFile::MySQLInit()
{
    m_MySQLThread.m_pLua = this;
    m_MySQLThread.m_Running = true;
    m_MySQLThread.m_Queries = 0;
    m_MySQLThread.m_MySSQLLock = lock_create();
    m_MySQLThread.pThread = thread_create(MySQLWorkerThread, &m_MySQLThread);
}

void CLuaFile::MySQLWorkerThread(void *pUser)
{
    CMySQLThread *pData = (CMySQLThread *)pUser;
    lock_wait(pData->m_MySSQLLock);
    while(pData->m_Running)
    {
        if (pData->m_pLua->m_lpQueries.GetSize() == 0)
        {
            thread_sleep(100);
        }
        if (pData->m_pLua->m_lpQueries.GetSize() > 0)
        {
            CQuery *pQuery = pData->m_pLua->m_lpQueries[0];
            pData->m_pLua->m_lpQueries.DeleteByIndex(0);

            CResults *pResult = new CResults();
            pResult->m_QueryId = pQuery->m_QueryId;
            if (mysql_real_query(&pData->m_pLua->m_MySQL, pQuery->m_pQuery, pQuery->m_Length) == 0)
            {
                MYSQL_RES *pMySQLResult = mysql_store_result(&pData->m_pLua->m_MySQL);
                if (pMySQLResult)
                {
                    int NumFields = mysql_num_fields(pMySQLResult);
                    MYSQL_ROW Row;
                    while ((Row = mysql_fetch_row(pMySQLResult)))
                    {
                        CRow *pRow = new CRow();

                        unsigned long *pLength = mysql_fetch_lengths(pMySQLResult);
                        for(int i = 0; i < NumFields; i++)
                        {
                            CField *pField = new CField();
                            MYSQL_FIELD *pMySQLField = mysql_fetch_field_direct(pMySQLResult, i);

                            if (pMySQLField->name_length > 0)
                            {
                                pField->m_pName = new char[pMySQLField->name_length + 1]; //null?
                                str_copy(pField->m_pName, pMySQLField->name, pMySQLField->name_length + 1);
                            }
                            /*enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                        MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                        MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                        MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                        MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
                        MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                        MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                        MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                        MYSQL_TYPE_BIT,
                        MYSQL_TYPE_NEWDECIMAL=246,
                        MYSQL_TYPE_ENUM=247,
                        MYSQL_TYPE_SET=248,
                        MYSQL_TYPE_TINY_BLOB=249,
                        MYSQL_TYPE_MEDIUM_BLOB=250,
                        MYSQL_TYPE_LONG_BLOB=251,
                        MYSQL_TYPE_BLOB=252,
                        MYSQL_TYPE_VAR_STRING=253,
                        MYSQL_TYPE_STRING=254,
                        MYSQL_TYPE_GEOMETRY=255,
                        MAX_NO_FIELD_TYPES
};
*/
                            //pField->m_Length = pMySQLField->length;
                            pField->m_Length = pLength[i]; //this is better?
                            if (pMySQLField->type == MYSQL_TYPE_TINY)
                            {
                                pField->m_Number = atol(Row[i]);
                                pField->m_Type = CField::TYPE_INTEGER;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_SHORT)
                            {
                                pField->m_Number = atol(Row[i]);
                                pField->m_Type = CField::TYPE_INTEGER;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_LONG)
                            {
                                pField->m_Number = atol(Row[i]);
                                pField->m_Type = CField::TYPE_INTEGER;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_LONGLONG)
                            {
                                // not supported by atol
                                // todo!
                                pField->m_Number = atol(Row[i]);
                                pField->m_Type = CField::TYPE_INTEGER;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_FLOAT)
                            {
                                pField->m_Float = atof(Row[i]);
                                pField->m_Type = CField::TYPE_FLOAT;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_DOUBLE)
                            {
                                pField->m_Float = atof(Row[i]);
                                pField->m_Type = CField::TYPE_FLOAT;
                            }
                            if (pMySQLField->type == MYSQL_TYPE_TIMESTAMP || pMySQLField->type == MYSQL_TYPE_DATE || pMySQLField->type == MYSQL_TYPE_TIME || pMySQLField->type == MYSQL_TYPE_DATETIME)
                            {
                            	if (pField->m_Length > 0)
                            	{
									pField->m_pData = new char[pField->m_Length];
									mem_copy(pField->m_pData, Row[i], pField->m_Length);
									pField->m_Type = CField::TYPE_DATA;
                            	}
                            }
                            if (pMySQLField->type == MYSQL_TYPE_STRING || pMySQLField->type == MYSQL_TYPE_VAR_STRING ||  pMySQLField->type == MYSQL_TYPE_BLOB)
                            {
                            	if (pField->m_Length > 0)
                            	{
									pField->m_pData = new char[pField->m_Length];
									mem_copy(pField->m_pData, Row[i], pField->m_Length);
									pField->m_Type = CField::TYPE_DATA;
                            	}
                            }
                            pRow->m_lpFields.Insert(pField);
                        }
                        pResult->m_lpRows.Insert(pRow);
                        pResult->m_Timestamp = time_get();
                    }
                    mysql_free_result(pMySQLResult);
                }
                pResult->m_Error = false;
            }
            else
            {
                dbg_msg("mysql", mysql_error(&pData->m_pLua->m_MySQL));
                pResult->m_Error = true;
            }

            pData->m_pLua->m_lpResults.Insert(pResult);
            pData->m_Queries--;
            delete []pQuery->m_pQuery;
            delete []pQuery;
        }
    }
    lock_release(pData->m_MySSQLLock);
}

int CLuaFile::MySQLFetchResults(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    int QueryId = lua_tointeger(L, 1);
    for (int i = 0; i < pSelf->m_lpResults.GetSize(); i++)
    {
        if (pSelf->m_lpResults[i]->m_QueryId == QueryId)
        {
            CResults *pResult = pSelf->m_lpResults[i];
            int Values = 0;
            lua_newtable(L);
            int ReturnTableIndex = lua_gettop(L);
            for (int RowId = 0; RowId < pResult->m_lpRows.GetSize(); RowId++)
            {
                CRow *pRow = pResult->m_lpRows[RowId];
                Values++;
                lua_pushinteger(L, Values);
                lua_newtable(L);
                int RowTableIndex = lua_gettop(L);
                for (int FieldId = 0; FieldId < pRow->m_lpFields.GetSize(); FieldId++)
                {
                    CField *pField = pRow->m_lpFields[FieldId];
                    if (pField->m_pName == 0)
                        continue;
                    lua_pushstring(L, pField->m_pName);
                    if (pField->m_Type == CField::TYPE_INTEGER)
                    {
                        lua_pushinteger(L, pField->m_Number);
                    }
                    else if (pField->m_Type == CField::TYPE_FLOAT)
                    {
                        lua_pushnumber(L, pField->m_Float);
                    }
                    else if (pField->m_Type == CField::TYPE_DATA)
                    {
                        lua_pushlstring(L, pField->m_pData, pField->m_Length);
                    }
                    else
                    {
                        lua_pushnil(L);
                    }
                    lua_settable(L, RowTableIndex);
                }
                lua_settable(L, ReturnTableIndex);
            }
            pSelf->MySQLFreeResult(i, QueryId);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::MySQLConnect(lua_State *L)
{
    LUA_FUNCTION_HEADER

    const char *pHost = 0;
    const char *pUsername = 0;
    const char *pPassword = 0;
    const char *pDatabase = 0;
    int Port = 0;
    if (lua_isstring(L, 1) && lua_isstring(L, 2), lua_isstring(L, 3))
    {
        pHost = lua_tostring(L, 1);
        pUsername = lua_tostring(L, 2);
        pPassword = lua_tostring(L, 3);
    }
    if (lua_isstring(L, 4))
    {
        pDatabase = lua_tostring(L, 4);
    }
    if (lua_isnumber(L, 5))
    {
        Port = lua_tointeger(L, 5);
    }

    if (pHost == 0 || pUsername == 0 || pPassword == 0)
        return 0;

    if (pSelf->m_MySQLConnected)
    {
        mysql_close(&pSelf->m_MySQL); //close conn
        pSelf->m_MySQLConnected = false;
    }
    if (mysql_init(&pSelf->m_MySQL) == 0)
    {
        dbg_msg("mysql", "fatal error while creating mysql object!");
    }
    if (mysql_real_connect(&pSelf->m_MySQL, pHost, pUsername, pPassword, pDatabase, Port, 0, 0) == 0)
    {
        dbg_msg("mysql", "fatal error while connecting!");
    }
    else
    {
        pSelf->m_MySQLConnected = true;
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

int CLuaFile::MySQLSelectDatabase(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_MySQLConnected && lua_isstring(L, 1))
    {
        if (mysql_select_db(&pSelf->m_MySQL, lua_tostring(L, 1)) == 0)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::MySQLIsConnected(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_MySQLConnected)
    {
        if (mysql_ping(&pSelf->m_MySQL) == 0)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
        else
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::MySQLEscapeString(lua_State *L)
{
    LUA_FUNCTION_HEADER

    size_t StrLength = 0;
    const char *pStr = 0;
    char *pEscapedStr = 0;
    if (pSelf->m_MySQLConnected && lua_isstring(L, 1))
    {
        pStr = lua_tolstring(L, 1, &StrLength);
        pEscapedStr = new char[StrLength * 2]; //should be enough
        mem_zero(pEscapedStr, StrLength * 2);

        unsigned long EscapedLength = mysql_real_escape_string(&pSelf->m_MySQL, pEscapedStr, pStr, StrLength);
        lua_pushlstring(L, pEscapedStr, EscapedLength);
        return 1;
    }
    return 0;
}

int CLuaFile::MySQLClose(lua_State *L)
{
    LUA_FUNCTION_HEADER
    mysql_close(&pSelf->m_MySQL); //close conn
    pSelf->m_MySQLConnected = false;
    return 0;
}

int CLuaFile::MySQLQuery(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if (pSelf->m_MySQLConnected == false)
        return 0;

    if (lua_isstring(L, 1))
    {
        size_t Size = 0;
        const char *pQueryString = lua_tolstring(L, 1, &Size);
        if (Size <= 0)
            return 0;

        pSelf->m_MySQLThread.m_Queries++;
        pSelf->m_IncrementalQueryId++;
        CQuery *pQuery = new CQuery();
        pQuery->m_pQuery = new char[Size + 1];
        mem_copy(pQuery->m_pQuery, (void *)pQueryString, Size);
        pQuery->m_pQuery[Size] = 0;
        pQuery->m_QueryId = pSelf->m_IncrementalQueryId;
        pQuery->m_Length = Size;
        pSelf->m_lpQueries.Insert(pQuery);
        lua_pushnumber(L, pQuery->m_QueryId);
        return 1;
    }
    return 0;
}

#endif
