/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
/*DGI:Doc-Gen-Info*/
/*DGI:Type:Client,Server*/
/*DGI:Exception:errorfunc*/


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
	void Tick();

    T *m_pLua;

	static int NetCreate(lua_State *L);
	static int NetConnect(lua_State *L);
	static int NetListen(lua_State *L);
	static int NetAccept(lua_State *L);
	static int NetClose(lua_State *L);
	static int NetSend(lua_State *L);
	static int NetRecv(lua_State *L);
	static int NetGetStatus(lua_State *L);
	static int NetGetRemoteAddr(lua_State *L);
};

#define LUA_FUNCTION_HEADER     lua_getglobal(L, "pLUA"); \
    T *pSelf = (T *)lua_touserdata(L, -1); \
    lua_Debug Frame; \
    lua_getstack(L, 1, &Frame); \
    lua_getinfo(L, "nlSf", &Frame); \
    ((void)(pSelf));

template <class T>
CLuaShared<T>::CLuaShared(T *pLua)
{
	lua_register(pLua->m_pLua, ToLower("NetCreate"), NetCreate);
	lua_register(pLua->m_pLua, ToLower("NetConnect"), NetConnect);
	lua_register(pLua->m_pLua, ToLower("NetListen"), NetListen);
	lua_register(pLua->m_pLua, ToLower("NetAccept"), NetAccept);
	lua_register(pLua->m_pLua, ToLower("NetClose"), NetClose);
	lua_register(pLua->m_pLua, ToLower("NetSend"), NetSend);
	lua_register(pLua->m_pLua, ToLower("NetRecv"), NetRecv);
	lua_register(pLua->m_pLua, ToLower("NetGetStatus"), NetGetStatus);
	lua_register(pLua->m_pLua, ToLower("NetGetRemoteAddr"), NetGetRemoteAddr);
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
    delete []pSocket;
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
void CLuaShared<T>::Tick()
{
	for (array<CLuaSocket *>::range r = m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_Type == LUANETTYPETCP && r.front()->m_pNetTCP)
            r.front()->m_pNetTCP->Tick();
		if (r.front()->m_Type == LUANETTYPEUDP && r.front()->m_pNetUDP)
            r.front()->m_pNetUDP->Tick();
	}
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
		pNewSocket->m_ID = pSelf->m_pLuaShared->m_SocketID++;
		pNewSocket->m_Type = LUANETTYPETCP;

		NETADDR BindAddr;
		mem_zero(&BindAddr, sizeof(BindAddr));
		if (lua_isstring(L, 2))
            net_addr_from_str(&BindAddr, lua_tostring(L, 2));
        else
            BindAddr.type = NETTYPE_IPV4;

		pNewSocket->m_pNetTCP->Open(BindAddr);

		pSelf->m_pLuaShared->m_lpSockets.add(pNewSocket);
        lua_pushinteger(L, pNewSocket->m_ID);
        return 1;
	}
	if (str_comp_nocase(lua_tostring(L, 1), "udp") == 0)
	{
		CLuaSocket *pNewSocket = new CLuaSocket();
		pNewSocket->m_pNetTCP = 0;
		pNewSocket->m_pNetUDP = new CNetUDP();
		pNewSocket->m_ID = pSelf->m_pLuaShared->m_SocketID++;
		pNewSocket->m_Type = LUANETTYPEUDP;

		NETADDR BindAddr;
		mem_zero(&BindAddr, sizeof(BindAddr));
		if (lua_isstring(L, 2))
            net_addr_from_str(&BindAddr, lua_tostring(L, 2));
        else
            BindAddr.type = NETTYPE_IPV4;

		pNewSocket->m_pNetUDP->Open(BindAddr);

		pSelf->m_pLuaShared->m_lpSockets.add(pNewSocket);
        lua_pushinteger(L, pNewSocket->m_ID);
        return 1;
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetConnect(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);

	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
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

template <class T>
int CLuaShared<T>::NetClose(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
            pSelf->m_pLuaShared->FreeSocket(r.front());
            pSelf->m_pLuaShared->m_lpSockets.remove(r.front());
            lua_pushboolean(L, 1);
            return 1;
		}
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetSend(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
			    if (!lua_isstring(L, 2))
                    return 0;
                size_t Size = 0;
                const char *pData = lua_tolstring(L, 2, &Size);
                r.front()->m_pNetTCP->Send(pData, Size);
                lua_pushboolean(L, 1);
                return 1;
			}
			else
			{
			    if (!lua_isstring(L, 2)) //addr
                    return 0;
			    if (!lua_isstring(L, 3)) //data
                    return 0;

                NETADDR Addr;
                mem_zero(&Addr, sizeof(NETADDR));
                Addr.type = NETTYPE_IPV4;
                net_addr_from_str(&Addr, lua_tostring(L , 2));
                size_t Size = 0;
                const char *pData = lua_tolstring(L, 3, &Size);
                r.front()->m_pNetUDP->Send(Addr, pData, Size);
                lua_pushboolean(L, 1);
                return 1;
			}
		}
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetListen(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
                r.front()->m_pNetTCP->Listen();
                lua_pushboolean(L, 1);
                return 1;
			}
			else
			{
                return 0;
			}
		}
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetAccept(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
			    CNetTCP *pOtherSocket = 0;
			    if (lua_isnumber(L, 2))
			    {
                    for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
                    {
                        if (r.front()->m_ID == lua_tointeger(L, 2))
                        {
                            if (r.front()->m_Type == LUANETTYPETCP)
                            {
                                pOtherSocket = r.front()->m_pNetTCP;
                                break;
                            }
                        }
                    }
			    }
                lua_pushinteger(L, r.front()->m_pNetTCP->Accept(pOtherSocket));
                return 1;
			}
			else
			{
                return 0;
			}
		}
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetRecv(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
			    int Size = 8192;
			    if (lua_isnumber(L, 2) && lua_tointeger(L, 2) > 0 && lua_tointeger(L, 2) < 65536)
                    Size = lua_tointeger(L, 2);

                char *pData = new char[Size];
                Size = r.front()->m_pNetTCP->Recv(pData, Size);
                lua_pushinteger(L, Size);
                lua_pushlstring(L, pData, Size);
                delete []pData;
                return 2;
			}
			else
			{
			    int Size = 8192;
			    if (lua_isnumber(L, 2) && lua_tointeger(L, 2) > 0 && lua_tointeger(L, 2) < 65536)
                    Size = lua_tointeger(L, 2);

                NETADDR RemoteAddr;
                mem_zero(&RemoteAddr, sizeof(NETADDR));
                char *pData = new char[Size];
                char aRemoteAddr[64];
                Size = r.front()->m_pNetUDP->Recv(&RemoteAddr, pData, Size);
			    net_addr_str(&RemoteAddr, aRemoteAddr, sizeof(aRemoteAddr), true);
                lua_pushinteger(L, Size);
                lua_pushlstring(L, pData, Size);
                lua_pushstring(L, aRemoteAddr);
                delete []pData;
                return 3;
			}
		}
	}
	return 0;
}

template <class T>
int CLuaShared<T>::NetGetStatus(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
                lua_pushinteger(L, r.front()->m_pNetTCP->GetStatus());
                return 1;
			}
			else
                break;
		}
	}
    return 0;
}

template <class T>
int CLuaShared<T>::NetGetRemoteAddr(lua_State *L)
{
	LUA_FUNCTION_HEADER
	if (!lua_isnumber(L, 1))
		return 0;
	int SocketID = lua_tonumber(L, 1);
	for (array<CLuaSocket *>::range r = pSelf->m_pLuaShared->m_lpSockets.all(); !r.empty(); r.pop_front())
	{
		if (r.front()->m_ID == SocketID)
		{
			if (r.front()->m_Type == LUANETTYPETCP)
			{
			    char aBuf[64];
			    NETADDR RemoteAddr = r.front()->m_pNetTCP->GetRemoteAddr();
			    net_addr_str(&RemoteAddr, aBuf, sizeof(aBuf), true);
                lua_pushstring(L, aBuf);
                return 1;
			}
			else
                break;
		}
	}
    return 0;
}

#undef LUA_FUNCTION_HEADER

#endif
