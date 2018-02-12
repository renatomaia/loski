#include "netlib.h"

#include <lua.h> /* to copy error messages to Lua */


static int inet_aton(const char *cp, struct in_addr *inp)
{
	unsigned int a = 0, b = 0, c = 0, d = 0;
	int n = 0, r;
	unsigned long int addr = 0;
	r = sscanf(cp, "%u.%u.%u.%u%n", &a, &b, &c, &d, &n);
	if (r == 0 || n == 0) return 0;
	cp += n;
	if (*cp) return 0;
	if (a > 255 || b > 255 || c > 255 || d > 255) return 0;
	if (inp) {
		addr += a; addr <<= 8;
		addr += b; addr <<= 8;
		addr += c; addr <<= 8;
		addr += d;
		inp->s_addr = htonl(addr);
	}
	return 1;
}

LOSIDRV_API int losi_opennetwork(losi_NetDriver *drv) {
	WSADATA info;
	WORD version = MAKEWORD(2, 0); 
	int err = WSAStartup(version, &info);
	if (err != 0) return err;
	if ((LOBYTE(info.wVersion) != 2 || HIBYTE(info.wVersion) != 0) &&
	    (LOBYTE(info.wVersion) != 1 || HIBYTE(info.wVersion) != 1)) {
		WSACleanup();
		return WSAVERNOTSUPPORTED; 
	}
	return 0;
}

LOSIDRV_API int losi_closenetwork(losi_NetDriver *drv) {
	WSACleanup();
	return 0;
}

LOSIDRV_API int losi_addresserror(int error, lua_State *L) {
	switch (error) {
		case WSAHOST_NOT_FOUND: lua_pushstring(L, "host unknown"); break;
		case WSATRY_AGAIN: lua_pushstring(L, "unfulfilled"); break;
		case WSANO_RECOVERY: lua_pushstring(L, "address failed"); break;
		case WSANO_DATA: lua_pushstring(L, "address unavailable"); break;
		case WSAEAFNOSUPPORT: lua_pushstring(L, "unsupported"); break;
		default: lua_pushstring(L, "unspecified error"); break;
	}
	return 0;
}

LOSIDRV_API int losi_resolveaddress(losi_NetDriver *drv,
                                      losi_Address *address,
                                      const char *host,
                                      unsigned short port) {
	struct sockaddr_in *addr_in = (struct sockaddr_in *)address;
	memset(address, 0, sizeof(losi_Address));
	/* address is either wildcard or a valid ip address */
	addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
	addr_in->sin_port = htons(port);
	addr_in->sin_family = AF_INET;
	if (strcmp(host, "*") && !inet_aton(host, &addr_in->sin_addr)) {
		struct hostent *hp = NULL;
		struct in_addr **addr;
		hp = gethostbyname(host);
		if (hp == NULL) return h_errno;
		addr = (struct in_addr **) hp->h_addr_list;
		memcpy(&addr_in->sin_addr, *addr, sizeof(struct in_addr));
	}
	return 0;
}

LOSIDRV_API int losi_extractaddress(losi_NetDriver *drv,
                                      const losi_Address *address,
                                      const char **host,
                                      unsigned short *port) {
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (addr->sin_family != AF_INET) return WSAEAFNOSUPPORT;
	*host = inet_ntoa(addr->sin_addr);
	*port = ntohs(addr->sin_port);
	return 0;
}

LOSIDRV_API int losi_socketerror(int error, lua_State *L) {
	switch (error) {
		/* errors due to internal factors */
		case WSAEALREADY:
		case WSAEWOULDBLOCK:
		case WSAEINPROGRESS: lua_pushliteral(L, "unfulfilled"); break;
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAENOTCONN: lua_pushliteral(L, "disconnected"); break;
		case WSAEISCONN: lua_pushliteral(L, "connected"); break;
		/* errors due to external factors */
		case WSAECONNREFUSED: lua_pushliteral(L, "refused"); break;
		case WSAEACCES: lua_pushliteral(L, "access denied"); break;
		case WSAEADDRINUSE: lua_pushliteral(L, "address used"); break;
		case WSAEADDRNOTAVAIL: lua_pushliteral(L, "address unavailable"); break;
		case WSAEHOSTUNREACH: lua_pushliteral(L, "host unreachable"); break;
		case WSAENETUNREACH: lua_pushliteral(L, "network unreachable"); break;
		case WSAENETRESET: lua_pushliteral(L, "network reset"); break;
		case WSAENETDOWN: lua_pushliteral(L, "network down"); break;
		case WSAEDISCON: /* graceful shutdown in progress - win32 */
		case WSAESHUTDOWN: lua_pushliteral(L, "shutdown"); break; /* win32 */
		case WSAEHOSTDOWN: lua_pushliteral(L, "host down"); break; /* win32 */
		case WSASYSNOTREADY: lua_pushliteral(L, "network unavailable"); break; /* win32 */
		/* errors due to misuse */
		case WSAEDESTADDRREQ: lua_pushliteral(L, "address required"); break;
		case WSAEMSGSIZE: lua_pushliteral(L, "message too long"); break;
		/* system errors */
		case WSAENOBUFS: lua_pushliteral(L, "no buffer"); break;
		case WSAETIMEDOUT: lua_pushliteral(L, "timeout"); break;
		case WSAEINTR: lua_pushliteral(L, "interrupted"); break;
		case WSAEMFILE:
		case WSAEPROCLIM: lua_pushliteral(L, "no resources"); break; /* win32 */
		/* unexpected errors (probably a bug in the library) */
		case WSAEBADF: /* bad file descriptor */
		case WSAEFAULT: /* bad address */
		case WSANOTINITIALISED: /* initialization not performed - win32 */
		case WSAEINVALIDPROCTABLE: /* invalid service provider procedure table - win32 */
		case WSAEPROVIDERFAILEDINIT: /* service provider failed to initialize - win32 */
			lua_pushstring(L, "invalid operation"); break;
		case WSAEINVAL: /* invalid argument */
		case WSAENOTSOCK: /* socket operation on nonsocket */
		case WSAEPROTOTYPE: /* protocol wrong type for socket */
		case WSAENOPROTOOPT: /* the option is not supported by the protocol. */
		case WSAEPROTONOSUPPORT: /* protocol not supported */
		case WSAEAFNOSUPPORT: /* address family not supported */
		case WSAEOPNOTSUPP: /* operation not supported on socket */
		case WSAEPFNOSUPPORT: /* protocol family not supported - win32 */
		case WSAESOCKTNOSUPPORT: /* socket type not supported - win32 */
		case WSAVERNOTSUPPORTED: /* winsock.dll version out of range - win32 */
			lua_pushstring(L, "unsupported"); break;
		default: lua_pushstring(L, "unspecified error"); break;
	}
	return 0;
}

LOSIDRV_API int losi_createsocket(losi_NetDriver *drv,
                                    losi_Socket *sock,
                                    losi_SocketType type) {
	int kind;
	switch (type) {
		case LOSI_SOCKTYPE_DGRM:
			kind = SOCK_DGRAM;
			break;
		default:
			kind = SOCK_STREAM;
			break;
	}
	sock->id = socket(AF_INET, kind, 0);
	if (sock->id == INVALID_SOCKET) return WSAGetLastError();
	sock->blocking = 1;
	return 0;
}

struct OptionInfo {
	int level;
	int name;
};
static struct OptionInfo optinfo[] = {
	/* BASESOCKET */
	{0, 0},
	{SOL_SOCKET, SO_REUSEADDR},
	{SOL_SOCKET, SO_DONTROUTE},
	/* CONNSOCKET */
	{0, 0},
	{SOL_SOCKET, SO_KEEPALIVE},
	{IPPROTO_TCP, TCP_NODELAY},
	/* DRGMSOCKET */
	{SOL_SOCKET, SO_BROADCAST}
};

LOSIDRV_API int losi_setsocketoption(losi_NetDriver *drv,
                                       losi_Socket *sock,
                                       losi_SocketOption option,
                                       int value)
{
	int res;
	switch (option) {
		case LOSI_SOCKOPT_BLOCKING: {
			u_long argp = value ? 0 : 1;
			res = ioctlsocket(sock->id, FIONBIO, &argp);
			if (res == 0) sock->blocking = value;
		} break;
		case LOSI_SOCKOPT_LINGER: {
			struct linger li;
			if (value > 0) {
				li.l_onoff = 1;
				li.l_linger = value;
			} else {
				li.l_onoff = 0;
				li.l_linger = 0;
			}
			res = setsockopt(sock->id, SOL_SOCKET, SO_LINGER,
			                 (const char *)&li, sizeof(li));
		} break;
		default:
			res = setsockopt(sock->id, optinfo[option].level, optinfo[option].name,
			                 (const char *)&value, sizeof(value));
			break;
	}
	if (res != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_getsocketoption(losi_NetDriver *drv,
                                       losi_Socket *sock,
                                       losi_SocketOption option,
                                       int *value)
{
	int res;
	switch (option) {
		case LOSI_SOCKOPT_BLOCKING: {
			*value = sock->blocking;
			res = 0;
		} break;
		case LOSI_SOCKOPT_LINGER: {
			struct linger li;
			int sz = sizeof(li);
			res = getsockopt(sock->id, SOL_SOCKET, SO_LINGER, (char *)&li, &sz);
			if (res == 0) *value = li.l_onoff ? li.l_linger : 0;
		} break;
		default: {
			int sz = sizeof(*value);
			res = getsockopt(sock->id, optinfo[option].level, optinfo[option].name,
			                 (char *)value, &sz);
		} break;
	}
	if (res != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_bindsocket(losi_NetDriver *drv,
                                  losi_Socket *sock,
                                  const losi_Address *address) {
	int res = bind(sock->id, address, sizeof(losi_Address));
	if (res != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_socketaddress(losi_NetDriver *drv,
                                     losi_Socket *sock,
                                     losi_Address *address,
                                     losi_SocketSite site) {
	int res;
	int len = sizeof(losi_Address);
	if (site == LOSI_LOCALSITE) res = getsockname(sock->id, address, &len);
	else                         res = getpeername(sock->id, address, &len);
	if (res != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_connectsocket(losi_NetDriver *drv,
                                     losi_Socket *sock,
                                     const losi_Address *address) {
	int res = connect(sock->id, address, sizeof(losi_Address));
	if (res != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_sendtosocket(losi_NetDriver *drv,
                                    losi_Socket *sock,
                                    const char *data,
                                    size_t size,
                                    size_t *bytes,
                                    const losi_Address *address) {
	int res;
	if (address) {
		res = sendto(sock->id, data, size, 0, address, sizeof(losi_Address));
	} else {
		res = send(sock->id, data, size, 0);
	}
	if (res == SOCKET_ERROR) return WSAGetLastError();
	*bytes = (size_t)res;
	return 0;
}

LOSIDRV_API int losi_recvfromsocket(losi_NetDriver *drv,
                                      losi_Socket *sock,
                                      char *buffer,
                                      size_t size,
                                      size_t *bytes,
                                      losi_Address *address) {
	int res;
	if (address) {
		int len = sizeof(losi_Address);
		memset(address, 0, len);
		res = recvfrom(sock->id, buffer, size, 0, address, &len);
	} else {
		res = recv(sock->id, buffer, size, 0);
	}
	if (res == 0) return WSAECONNABORTED;
	if (res == SOCKET_ERROR) return WSAGetLastError();
	*bytes = (size_t)res;
	return 0;
}

LOSIDRV_API int losi_shutdownsocket(losi_NetDriver *drv,
                                      losi_Socket *sock,
                                      losi_SocketSite site) {
	int how;
	switch (site) {
		case LOSI_LOCALSITE:
			how = 1 /* SD_SEND */;
			break;
		case LOSI_REMOTESITE:
			how = 0 /* SD_RECEIVE */;
			break;
		case LOSI_BOTHSITES:
			how = 2 /* SD_BOTH */;
			break;
	}
	if (shutdown(sock->id, how) != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_acceptsocket(losi_NetDriver *drv,
                                    losi_Socket *sock,
                                    losi_Socket *accepted,
                                    losi_Address *address) {
	int len = 0;
	if (address) {
		len = sizeof(losi_Address);
		memset(address, 0, len);
	}
	accepted->id = accept(sock->id, address, &len);
	if (accepted->id == INVALID_SOCKET) return WSAGetLastError();
	accepted->blocking = 1;
	return 0;
}

LOSIDRV_API int losi_listensocket(losi_NetDriver *drv,
                                    losi_Socket *sock,
                                    int backlog) {
	if (listen(sock->id, backlog) != 0) return WSAGetLastError();
	return 0;
}

LOSIDRV_API int losi_closesocket(losi_NetDriver *drv,
                                   losi_Socket *sock) {
	if (closesocket(sock->id) != 0) return WSAGetLastError();
	return 0;
}
