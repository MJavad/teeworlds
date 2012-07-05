/*
** $Id: lmathlib.c,v 1.81 2012/05/18 17:47:53 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/


#include <stdlib.h>
#include <math.h>

#define lcryptolib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#define between(x, min, max) (x > min && x < max)

static int crypto_strhex (lua_State *L) {
  size_t size = 0;
  const char *string = luaL_checklstring(L, 1, &size);
  static const char hex[] = "0123456789ABCDEF";
  int i;
  luaL_Buffer b;
  char *p = luaL_buffinitsize(L, &b, size * 2);

  if (!string)
    return 0;
  for(i = 0; i < size; i++){
    p[i*2] = hex[((const unsigned char *)string)[i]>>4];
    p[i*2+1] = hex[((const unsigned char *)string)[i]&0xf];
  }

  luaL_pushresultsize(&b, size * 2);
  return 1;
}

static int crypto_hexstr (lua_State *L) {
  size_t size = 0;
  const char *string = luaL_checklstring(L, 1, &size);
  int i;
  luaL_Buffer b;
  char *p = luaL_buffinitsize(L, &b, size / 2);

  if (!string)
    return 0;
  for(i = 0; i < size / 2; i++){
    unsigned char c1 = tolower(((const unsigned char *)string)[i*2]);
    unsigned char c2 = tolower(((const unsigned char *)string)[i*2+1]);
    if ((between(c1, 47, 58) || between(c1, 96, 123)) && (between(c2, 47, 58) || between(c2, 96, 123)))
    {
      c1 = c1 - 48;
      if (c1 > 9)
        c1 = c1 - 49;
      c2 = c2 - 48;
      if (c2 > 9)
        c2 = c2 - 49;
      p[i] = c1 * 16 + c2;
    }
    else
      return 0;
  }

  luaL_pushresultsize(&b, size / 2);
  return 1;
}


static const luaL_Reg cryptolib[] = {
  //{"strbin",   crypto_strbin},
  //{"stroct",   crypto_stroct},
  //{"strdec",   crypto_stroct},
  {"strhex",   crypto_strhex},
  //{"strbase64",   crypto_strbase64},

  {"hexstr",   crypto_hexstr},
  {NULL, NULL}
};


/*
** Open crypto library
*/
LUAMOD_API int luaopen_crypto (lua_State *L) {
  luaL_newlib(L, cryptolib);
  /*lua_pushnumber(L, PI);
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, HUGE_VAL);
  lua_setfield(L, -2, "huge");*/
  return 1;
}

