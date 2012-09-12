#ifndef GAME_LUASHARED_H
#define GAME_LUASHARED_H

#include <engine/shared/network.h>
#include <base/tl/array.h>

extern "C" { // lua
    #define LUA_CORE /* make sure that we don't try to import these functions */
    #include <engine/external/lua/lua.h>
    #include <engine/external/lua/lualib.h> /* luaL_openlibs */
    #include <engine/external/lua/lauxlib.h> /* luaL_loadfile */
}

#define LUANETTYPETCP 1
#define LUANETTYPEUDP 2
struct CLuaSocket
{
	int m_ID;
	int m_Type;
	CNetTCP *m_pNetTCP;
	CNetUDP *m_pNetUDP;
};

class CLuaShared
{
	unsigned int m_SocketID;
	array<CLuaSocket *>m_lpSockets;
public:
	CLuaShared(lua_State *L);
	~CLuaShared();
	void FreeSocket(CLuaSocket *pSocket);
	void Clear();


	static int NetCreate(lua_State *L);
	static int NetConnect(lua_State *L);
	static int NetListen(lua_State *L);
	static int NetAccept(lua_State *L);
	static int NetClose(lua_State *L);
	static int NetSend(lua_State *L);
	static int NetRecv(lua_State *L);
	static int NetGetStatus(lua_State *L);
};
#endif
