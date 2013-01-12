//some macros and helperfunctions
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef GAME_LUAGLOBAL_H
#define GAME_LUAGLOBAL_H


#if defined(LUA_USE_GMTIME_R)

#define l_gmtime(t,r)		gmtime_r(t,r)
#define l_localtime(t,r)	localtime_r(t,r)

#elif !defined(l_gmtime)

#define l_gmtime(t,r)		((void)r, gmtime(t))
#define l_localtime(t,r)  	((void)r, localtime(t))

#endif

#define LUA_FUNCTION_HEADER     lua_getglobal(L, "pLUA"); \
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1); \
    lua_Debug Frame; \
    lua_getstack(L, 1, &Frame); \
    lua_getinfo(L, "nlSf", &Frame); \
    ((void)(pSelf));

#if !defined(LUA_USE_POSIX)
#define LUA_STRFTIMEOPTIONS	{ "aAbBcdHIjmMpSUwWxXyYz%", "" }
#else
#define LUA_STRFTIMEOPTIONS \
	{ "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%", "" \
	  "", "E", "cCxXyY",  \
	  "O", "deHImMSuUVwWy" }
#endif

static void setfield (lua_State *L, const char *key, int value)
{
    lua_pushinteger(L, value);
    lua_setfield(L, -2, key);
}

static void setboolfield (lua_State *L, const char *key, int value)
{
    if (value < 0)  /* undefined? */
        return;  /* does not set field */
    lua_pushboolean(L, value);
    lua_setfield(L, -2, key);
}

static int getboolfield (lua_State *L, const char *key)
{
    int res;
    lua_getfield(L, -1, key);
    res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return res;
}


static int getfield (lua_State *L, const char *key, int d)
{
    int res, isnum;
    lua_getfield(L, -1, key);
    res = (int)lua_tointegerx(L, -1, &isnum);
    if (!isnum)
    {
        if (d < 0)
            return luaL_error(L, "field " LUA_QS " missing in date table", key);
        res = d;
    }
    lua_pop(L, 1);
    return res;
}


static const char *checkoption (lua_State *L, const char *conv, char *buff)
{
    static const char *const options[] = LUA_STRFTIMEOPTIONS;
    unsigned int i;
    for (i = 0; i < sizeof(options)/sizeof(options[0]); i += 2)
    {
        if (*conv != '\0' && strchr(options[i], *conv) != NULL)
        {
            buff[1] = *conv;
            if (*options[i + 1] == '\0')    /* one-char conversion specifier? */
            {
                buff[2] = '\0';  /* end buffer */
                return conv + 1;
            }
            else if (*(conv + 1) != '\0' &&
                     strchr(options[i + 1], *(conv + 1)) != NULL)
            {
                buff[2] = *(conv + 1);  /* valid two-char conversion specifier */
                buff[3] = '\0';  /* end buffer */
                return conv + 2;
            }
        }
    }
    luaL_argerror(L, 1,
                  lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
    return conv;  /* to avoid warnings */
}

#endif
