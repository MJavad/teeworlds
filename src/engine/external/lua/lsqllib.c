/*
** $Id: lmathlib.c,v 1.67.1.1 2007/12/27 13:02:25 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lsqllib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include <engine/external/sqlite/sqlite3.h>

#define MAXDATABASES 128
struct CDatabase {
    int m_Used;
    sqlite3 *m_pHandle;
} Databases[MAXDATABASES];

static int sql_create (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    const char *pFilename = luaL_checkstring(L, 1);
    int i = 0;
    for (; i < MAXDATABASES; i++)
    {
        if (!Databases[i].m_Used)
        {
            int Ret = sqlite3_open(pFilename, &Databases[i].m_pHandle);
            if (Ret != SQLITE_OK)
                return 0;
            Databases[i].m_Used = 1;
            lua_pushinteger(L, i);
            return 1;
        }

    }
    return 0;
}

static int sql_query (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    int i = (int)luaL_checkinteger(L, 1);
    if (Databases[i].m_Used)
    {
        int ReturnTableIndex = 0;
        int Values = 0;
        size_t Size = 0;
        const char *pQuery = luaL_checklstring(L, 2, &Size);
        sqlite3_stmt *pStatement = 0;
        int Ret = sqlite3_prepare(Databases[i].m_pHandle, pQuery, Size, &pStatement, 0);
        if (Ret != SQLITE_OK)
        {
            printf("%s\n", sqlite3_errmsg(Databases[i].m_pHandle));
            return 0;
        }
        lua_newtable(L);
        ReturnTableIndex = lua_gettop(L);
        while ((Ret = sqlite3_step(pStatement)) != SQLITE_BUSY && Ret != SQLITE_DONE && Ret != SQLITE_ERROR && Ret != SQLITE_MISUSE)
        {
            if (Ret == SQLITE_ROW)
            {
                int iCol = 0;
                int RowTableIndex = 0;
                Values++;
                lua_pushinteger(L, Values);
                lua_newtable(L);
                RowTableIndex = lua_gettop(L);
                for (iCol; iCol < sqlite3_column_count(pStatement); iCol++)
                {
                    int Type = sqlite3_column_type(pStatement, iCol);
                    lua_pushstring(L, sqlite3_column_name(pStatement, iCol));
                    if (Type == SQLITE_INTEGER)
                    {
                        lua_pushinteger(L, sqlite3_column_int(pStatement, iCol));
                    }
                    else if (Type == SQLITE_FLOAT)
                    {
                        lua_pushinteger(L, sqlite3_column_double(pStatement, iCol));
                    }
                    else if (Type == SQLITE_TEXT)
                    {
                        const char *pTmp = sqlite3_column_text(pStatement, iCol);
                        size_t Size = sqlite3_column_bytes(pStatement, iCol);
                        lua_pushlstring(L, pTmp, Size);
                    }
                    else if (Type == SQLITE_BLOB)
                    {
                        const char *pTmp = sqlite3_column_blob(pStatement, iCol);
                        size_t Size = sqlite3_column_bytes(pStatement, iCol);
                        lua_pushlstring(L, pTmp, Size);
                    }
                    else
                    {
                        lua_pushnil(L);
                    }
                    lua_settable(L, RowTableIndex);
                }
                lua_settable(L, ReturnTableIndex);
            }
        }
        sqlite3_finalize(pStatement);
        return 1;
    }
    return 0;
}

static int sql_close (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    int i = (int)luaL_checkinteger(L, 1);
    if (n != 1)
        return 0;
    if (Databases[i].m_Used)
    {
        sqlite3_close(Databases[i].m_pHandle);
        Databases[i].m_Used = 0;
    }
    return 0;
}

static const luaL_Reg sqllib[] =
{
  {"create",   sql_create},
  {"query",   sql_query},
  {"close",   sql_close},
  {NULL, NULL}
};


/*
** Open net library
*/
LUALIB_API int luaopen_sql (lua_State *L) {
    luaL_newlib(L, sqllib);
    mem_zero(Databases, sizeof(Databases));

    return 1;
}
