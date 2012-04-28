/*
** $Id: lbitlib.c,v 1.67.1.1 2007/12/27 13:02:25 roberto Exp $
** Standard bitwise-operator library
** See Copyright Notice in lua.h
*/


#include <stdlib.h>

#define lbitlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

static int bit_xor (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 2)
  {
    lua_pushnumber(L, (int)luaL_checknumber(L, 1) ^ (int)luaL_checknumber(L, 2));
    return 1;
  }
  return 0;
}

static int bit_and (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 2)
  {
    lua_pushnumber(L, (int)luaL_checknumber(L, 1) & (int)luaL_checknumber(L, 2));
    return 1;
  }
  return 0;
}

static int bit_or (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 2)
  {
    lua_pushnumber(L, (int)luaL_checknumber(L, 1) | (int)luaL_checknumber(L, 2));
    return 1;
  }
  return 0;
}

static int bit_not (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 1)
  {
    lua_pushnumber(L, ~(int)luaL_checknumber(L, 1));
    return 1;
  }
  return 0;
}

static int bit_lshift (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 2)
  {
    lua_pushnumber(L, (int)luaL_checknumber(L, 1) << (int)luaL_checknumber(L, 2));
    return 1;
  }
  return 0;
}

static int bit_rshift (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  if (n == 2)
  {
    lua_pushnumber(L, (int)luaL_checknumber(L, 1) >> (int)luaL_checknumber(L, 2));
    return 1;
  }
  return 0;
}


static const luaL_Reg bitlib[] = {
  {"xor",   bit_xor},
  {"and",   bit_and},
  {"or",   bit_or},
  {"not",   bit_not},
  {"lshift",   bit_lshift},
  {"rshift",   bit_rshift},
  {NULL, NULL}
};


/*
** Open bit library
*/
LUALIB_API int luaopen_bit (lua_State *L) {
  luaL_register(L, LUA_BITLIBNAME, bitlib);
  return 1;
}


