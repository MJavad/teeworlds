#ifndef GAME_LUASHARED_H
#define GAME_LUASHARED_H

#include <base/tl/array.h>


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