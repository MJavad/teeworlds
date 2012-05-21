/*
** $Id: lmathlib.c,v 1.67.1.1 2007/12/27 13:02:25 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lnetlib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include <base/system.h>

#define SOCKETTCP 1
#define SOCKETUDP 2
#define MAXSOCKETS 128
struct CSocket {
    int m_Type;
    NETSOCKET m_Socket;
    char m_Used;
    char m_Connected;
    char *m_pSendBuffer;
    int m_SendBufferSize;
} Sockets[MAXSOCKETS];

static int net_create (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    lua_Number type = luaL_checknumber(L, 1);
    int i = 0;
    if (type != SOCKETTCP && type != SOCKETUDP)
    {
        return 0;
    }
    for (i = 0; i < MAXSOCKETS; i++)
    {
        if (!Sockets[i].m_Used)
        {
            NETADDR BindAddr;
            mem_zero(&BindAddr, sizeof(NETADDR));
            Sockets[i].m_Used = 1;
            Sockets[i].m_Type = type;
            Sockets[i].m_Connected = 0;
            Sockets[i].m_pSendBuffer = 0;
            Sockets[i].m_SendBufferSize = 0;
            if (n > 1)
            {
                net_addr_from_str(&BindAddr, luaL_checkstring(L, 2));
            }
            if (type == SOCKETUDP)
                Sockets[i].m_Socket = net_udp_create(BindAddr);
            if (type == SOCKETTCP)
                Sockets[i].m_Socket = net_tcp_create(BindAddr);
            lua_pushinteger(L, i);
            return 1;
        }
    }
    return 0;
}

static int net_close (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    lua_Number socketid = luaL_checknumber(L, 1);
    if (!Sockets[(int)socketid].m_Used)
        return 0;
    if (Sockets[(int)socketid].m_Type == SOCKETTCP)
        net_tcp_close(Sockets[(int)socketid].m_Socket);
    if (Sockets[(int)socketid].m_Type == SOCKETUDP)
        net_udp_close(Sockets[(int)socketid].m_Socket);
    if (Sockets[(int)socketid].m_pSendBuffer)
        mem_free(Sockets[(int)socketid].m_pSendBuffer);
    Sockets[(int)socketid].m_Used = 0;
    return 0;
}

static int net_send (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    lua_Number socketid = luaL_checknumber(L, 1);
    if (!Sockets[(int)socketid].m_Used)
        return 0;
    if (Sockets[(int)socketid].m_Type == SOCKETTCP && n == 2)
    {
        int Size = 0;
        const char *pBuf = lua_tolstring(L, 2, &Size);

        char *pTmp = mem_alloc(Sockets[(int)socketid].m_SendBufferSize + Size, 0);
        mem_copy(pTmp, Sockets[(int)socketid].m_pSendBuffer, Sockets[(int)socketid].m_SendBufferSize);
        mem_copy(pTmp + Sockets[(int)socketid].m_SendBufferSize, pBuf, Size);
        Sockets[(int)socketid].m_SendBufferSize = Sockets[(int)socketid].m_SendBufferSize + Size;
        mem_free(Sockets[(int)socketid].m_pSendBuffer);
        Sockets[(int)socketid].m_pSendBuffer = pTmp;
    }
    if (Sockets[(int)socketid].m_Type == SOCKETUDP && n == 3)
    {
        NETADDR Addr;
        int Size = 0;
        const char *pTmp = lua_tolstring(L, 2, &Size);
        if (lua_tostring(L, 3) && net_addr_from_str(&Addr, lua_tostring(L, 3)) == 0)
            net_udp_send(Sockets[(int)socketid].m_Socket, &Addr, pTmp, Size);
    }
    return 0;
}

static int net_recv (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    lua_Number socketid = luaL_checknumber(L, 1);
    lua_Number maxsize = 8192;
    int size = 0;
    char aTmp[65536];
    if (n > 1)
        maxsize = luaL_checknumber(L, 2);
    if (!Sockets[(int)socketid].m_Used)
        return 0;
    if (Sockets[(int)socketid].m_Type == SOCKETTCP)
    {
        size = net_tcp_recv(Sockets[(int)socketid].m_Socket, aTmp, (int)maxsize);
        lua_pushlstring(L, aTmp, size);
        lua_pushinteger(L, size);
        return 2;
    }
    if (Sockets[(int)socketid].m_Type == SOCKETUDP)
    {
        NETADDR Addr;
        char AddrStr[256];
        size = net_udp_recv(Sockets[(int)socketid].m_Socket, &Addr, aTmp, (int)maxsize);
        if (size < 1)
            return 0;
        net_addr_str(&Addr, AddrStr, sizeof(AddrStr), 1);
        lua_pushlstring(L, aTmp, size);
        lua_pushinteger(L, size);
        lua_pushstring(L, AddrStr);
        return 3;
    }
    return 0;
}

static int net_flush (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    lua_Number socketid = luaL_checknumber(L, 1);
    if (Sockets[(int)socketid].m_Used && Sockets[(int)socketid].m_pSendBuffer)
    {
        if (Sockets[(int)socketid].m_Type == SOCKETTCP)
        {
            int Size = net_tcp_send(Sockets[(int)socketid].m_Socket, Sockets[(int)socketid].m_pSendBuffer, Sockets[(int)socketid].m_SendBufferSize);
            if (Size < Sockets[(int)socketid].m_SendBufferSize)
            {
                char *pTmp = mem_alloc(Sockets[(int)socketid].m_SendBufferSize - Size, 0);
                mem_copy(pTmp, Sockets[(int)socketid].m_pSendBuffer + Size, Sockets[(int)socketid].m_SendBufferSize - Size);
                Sockets[(int)socketid].m_SendBufferSize = Sockets[(int)socketid].m_SendBufferSize - Size;
                mem_free(Sockets[(int)socketid].m_pSendBuffer);
                Sockets[(int)socketid].m_pSendBuffer = pTmp;
            }
        }
    }
    return 0;
}

static const luaL_Reg netlib[] =
{
  {"create",   net_create},
  {"close",   net_close},
  {"send",   net_send},
  {"recv",   net_recv},
  {"flush",   net_flush},
  {NULL, NULL}
};


/*
** Open net library
*/
LUALIB_API int luaopen_net (lua_State *L) {
    luaL_newlib(L, netlib);
    lua_pushnumber(L, SOCKETTCP);
    lua_setfield(L, -2, "SOCKETTCP");
    lua_pushnumber(L, SOCKETUDP);
    lua_setfield(L, -2, "SOCKETUDP");

    mem_zero(Sockets, sizeof(Sockets));

    return 1;
}


