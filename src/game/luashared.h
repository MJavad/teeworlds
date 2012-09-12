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


template <class T>
class CLuaShared
{
	unsigned int m_SocketID;
	array<CLuaSocket *>m_lpSockets;
public:
	CLuaShared(T *pLua);
	~CLuaShared();
	void FreeSocket(CLuaSocket *pSocket);
	void Clear();

    T *m_pLua;

	static int NetCreate(lua_State *L);
	static int NetConnect(lua_State *L);
	static int NetListen(lua_State *L);
	static int NetAccept(lua_State *L);
	static int NetClose(lua_State *L);
	static int NetSend(lua_State *L);
	static int NetRecv(lua_State *L);
	static int NetGetStatus(lua_State *L);
};

#define LUA_FUNCTION_HEADER     lua_getglobal(L, "pLUA"); \
    T *pSelf = (T *)lua_touserdata(L, -1); \
    lua_Debug Frame; \
    lua_getstack(L, 1, &Frame); \
    lua_getinfo(L, "nlSf", &Frame); \
    ((void)(pSelf));

template <class T>
CLuaShared<T>::CLuaShared(lua_State *L)
{
	lua_register(L, ToLower("NetCreate"), NetCreate);
	lua_register(L, ToLower("NetConnect"), NetConnect);
	/*lua_register(L, ToLower("NetClose"), NetClose);
	lua_register(L, ToLower("NetSend"), NetClose);
	lua_register(L, ToLower("NetRecv"), NetRecv);
	lua_register(L, ToLower("NetGetStatus"), NetGetStatus);*/
}

template <class T>
CLuaShared<T>::~CLuaShared()
{
	Clear();
}

template <class T>
void CLuaShared<T>::FreeSocket(CLuaSocket *pSocket)
{
	if (pSocket->m_Type == LUANETTYPETCP && pSocket->m_pNetTCP)
	{
		delete []pSocket->m_pNetTCP;
	}
	if (pSocket->m_Type == LUANETTYPEUDP && pSocket->m_pNetUDP)
	{
		delete []pSocket->m_pNetUDP;
	}
}

template <class T>
void CLuaShared<T>::Clear()
{
	for (array<CLuaSocket *>::range r = m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		FreeSocket(r.front());
	}
	m_lpSockets.clear();
}


template <class T>
int CLuaShared<T>::NetCreate(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isstring(L, 1))
		return 0;
	if (str_comp_nocase(lua_tostring(L, 1), "tcp") == 0)
	{
		CLuaSocket *pNewSocket = new CLuaSocket();
		pNewSocket->m_pNetTCP = new CNetTCP();
		pNewSocket->m_pNetUDP = 0;
		pNewSocket->m_ID = pSelf->m_SocketID++;
		pNewSocket->m_Type = LUANETTYPETCP;

		NETADDR BindAddr;
		mem_zero(&BindAddr, sizeof(BindAddr));
		net_addr_from_str(&BindAddr, lua_tostring(L, 2));

		pNewSocket->m_pNetTCP->Open(BindAddr);

		pSelf->m_lpSockets.insert(pNewSocket);
	}
	if (str_comp_nocase(lua_tostring(L, 1), "udp") == 0)
	{
		CLuaSocket *pNewSocket = new CLuaSocket();
		pNewSocket->m_pNetTCP = 0;
		pNewSocket->m_pNetUDP = new CNetUDP();
		pNewSocket->m_ID = pSelf->m_SocketID++;
		pNewSocket->m_Type = LUANETTYPEUDP;

		NETADDR BindAddr;
		mem_zero(&BindAddr, sizeof(BindAddr));
		net_addr_from_str(&BindAddr, lua_tostring(L, 2));

		//pNewSocket->m_pNetUDP->Open(BindAddr);

		pSelf->m_lpSockets.insert(pNewSocket);
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetConnect(lua_State *L)
{

	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);

	for (array<CLuaSocket *>::range r = pSelf->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID())
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
				if (!lua_isstring(L, 2))
					return 0;
				NETADDR ConnAddr;
				net_addr_from_str(&ConnAddr, lua_tostring(L, 2));
				r.front()->m_pNetTCP->Connect(ConnAddr);

				lua_pushboolean(L, 1);
				return 1;
			}
			break;
		}
	}
	return 0;
}




#endif
