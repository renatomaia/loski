#include "netlib.h"

#include <lua.h> /* to copy error messages to Lua */
#include <errno.h>


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


#include <signal.h> /* sigpipe handling */

LOSKIDRV_API int loski_opennetwork(loski_NetState *state)
{
	/* instals a handler to ignore sigpipe or it will crash us */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return errno;
	return 0;
}

LOSKIDRV_API int loski_closenetwork(loski_NetState *state)
{
	if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) return errno;
	return 0;
}


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


#include <string.h>
#include <arpa/inet.h>  /* IP addresses */

/* TODO: ASSERT(LOSKI_ADDRSIZE_IPV4 == sizeof(in_addr_t)) */

#define MAXLITERAL	16

LOSKIDRV_API int loski_initaddress(loski_NetState *state,
                                   loski_Address *address,
                                   loski_AddressType type)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (type != LOSKI_ADDRTYPE_IPV4) return 0;
	memset(address, 0, sizeof(loski_Address));
	addr->sin_family = AF_INET;
	return 1;
}

LOSKIDRV_API void loski_setaddrport(loski_NetState *state,
                                    loski_Address *address,
                                    loski_AddressPort port)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	addr->sin_port = htons(port);
}

LOSKIDRV_API void loski_setaddrbytes(loski_NetState *state,
                                     loski_Address *address,
                                     const char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	memcpy(&(addr->sin_addr.s_addr), data, LOSKI_ADDRSIZE_IPV4);
}

LOSKIDRV_API int loski_setaddrliteral(loski_NetState *state,
                                      loski_Address *address,
                                      const char *data,
                                      size_t sz)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	char b[MAXLITERAL];
	if (sz >= MAXLITERAL) return 0;  /* too long! */
	memcpy(b, data, sz);
	b[sz] = '\0';
	return inet_aton(b, &addr->sin_addr);
}

LOSKIDRV_API loski_AddressType loski_getaddrtype(loski_NetState *state,
                                                 loski_Address *address)
{
	return LOSKI_ADDRTYPE_IPV4;
}

LOSKIDRV_API loski_AddressPort loski_getaddrport(loski_NetState *state,
                                                 loski_Address *address)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return ntohs(addr->sin_port);
}

LOSKIDRV_API const char *loski_getaddrbytes(loski_NetState *state,
                                            loski_Address *address,
                                            size_t *sz)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (sz) *sz = LOSKI_ADDRSIZE_IPV4;
	return (const char *)(&(addr->sin_addr.s_addr));
}

LOSKIDRV_API const char *loski_getaddrliteral(loski_NetState *state,
                                              loski_Address *address,
                                              char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return inet_ntoa(addr->sin_addr);
}


/*****************************************************************************
 * Names *********************************************************************
 *****************************************************************************/


//#include <netdb.h> /* gethostbyname and gethostbyaddr functions */
//
//#define LOSKI_EAFNOSUPPORT (-1)
//
//LOSKIDRV_API int loski_addresserror(int error, lua_State *L)
//{
//	switch (error) {
//		case HOST_NOT_FOUND: lua_pushstring(L, "host unknown"); break;
//		case TRY_AGAIN: lua_pushstring(L, "unfulfilled"); break;
//		case NO_RECOVERY: lua_pushstring(L, "address failed"); break;
//		case NO_DATA: lua_pushstring(L, "address unavailable"); break;
//		case LOSKI_EAFNOSUPPORT: lua_pushstring(L, "unsupported"); break;
//		default: lua_pushstring(L, "unspecified error"); break;
//	}
//	return 0;
//}
//
//LOSKIDRV_API int loski_resolveaddress(loski_NetState *state,
//                                      loski_Address *address,
//                                      const char *host,
//                                      unsigned short port)
//{
//	struct sockaddr_in *addr_in = (struct sockaddr_in *)address;
//	memset(address, 0, sizeof(loski_Address));
//	/* address is either wildcard or a valid ip address */
//	addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
//	addr_in->sin_port = htons(port);
//	addr_in->sin_family = AF_INET;
//	if (strcmp(host, "*") && !inet_aton(host, &addr_in->sin_addr)) {
//		struct hostent *hp = NULL;
//		struct in_addr **addr;
//		hp = gethostbyname(host);
//		if (hp == NULL) return h_errno;
//		addr = (struct in_addr **) hp->h_addr_list;
//		memcpy(&addr_in->sin_addr, *addr, sizeof(struct in_addr));
//	}
//	return 0;
//}


/*****************************************************************************
 * Sockets *******************************************************************
 *****************************************************************************/


#include <unistd.h>
#include <netinet/tcp.h> /* TCP options (nagle algorithm disable) */
#include <fcntl.h> /* fnctnl function and associated constants */

LOSKIDRV_API int loski_socketerror(int error, lua_State *L)
{
	if (error == EAGAIN) error = EWOULDBLOCK;
	switch (error) {
		/* errors due to internal factors */
		case EALREADY:
		case EWOULDBLOCK:
		case EINPROGRESS: lua_pushliteral(L, "unfulfilled"); break;
		case ECONNABORTED:
		case ECONNRESET:
		case EPIPE: /* not in win32 */
		case ENOTCONN: lua_pushliteral(L, "disconnected"); break;
		case EISCONN: lua_pushliteral(L, "connected"); break;
		/* errors due to external factors */
		case ECONNREFUSED: lua_pushliteral(L, "refused"); break;
		case EACCES: lua_pushliteral(L, "access denied"); break;
		case EADDRINUSE: lua_pushliteral(L, "address used"); break;
		case EADDRNOTAVAIL: lua_pushliteral(L, "address unavailable"); break;
		case EHOSTUNREACH: lua_pushliteral(L, "host unreachable"); break;
		case ENETUNREACH: lua_pushliteral(L, "network unreachable"); break;
		case ENETRESET: lua_pushliteral(L, "network reset"); break;
		case ENETDOWN: lua_pushliteral(L, "network down"); break;
		/* errors due to misuse */
		case EDESTADDRREQ: lua_pushliteral(L, "address required"); break;
		case EMSGSIZE: lua_pushliteral(L, "message too long"); break;
		/* system errors */
		case ENOBUFS: lua_pushliteral(L, "no buffer"); break;
		case ETIMEDOUT: lua_pushliteral(L, "timeout"); break;
		case EINTR: lua_pushliteral(L, "interrupted"); break;
		case EMFILE:
		case ENFILE: lua_pushliteral(L, "no resources"); break; /* not in win32 */
		case ENOMEM: lua_pushliteral(L, "no memory"); break; /* not in win32 */
		case EIO: lua_pushliteral(L, "system error"); break; /* not in win32 */
		/* unexpected errors (probably a bug in the library) */
		case EBADF: /* bad file descriptor */
		case EFAULT: /* bad address */
		case EDOM: /* numeric argument out of domain of function */
		case EISDIR: /* is a directory. */
		case ENOTDIR: /* is not a directory. */
		case ELOOP: /* too many levels of symbolic links. */
		case ENAMETOOLONG: /* filename too long. */
		case ENOENT: /* no such file or directory. */
		case EROFS: /* read-only file system. */
			lua_pushstring(L, "invalid operation"); break;
		case EINVAL: /* invalid argument */
		case ENOTSOCK: /* socket operation on nonsocket */
		case EPROTOTYPE: /* protocol wrong type for socket */
		case ENOPROTOOPT: /* the option is not supported by the protocol. */
		case EPROTONOSUPPORT: /* protocol not supported */
		case EAFNOSUPPORT: /* address family not supported */
		case EOPNOTSUPP: /* operation not supported on socket */
			lua_pushstring(L, "unsupported"); break;
		default: lua_pushstring(L, "unspecified error"); break;
	}
	return 0;
}

LOSKIDRV_API int loski_createsocket(loski_NetState *state,
                                    loski_Socket *sock,
                                    loski_SocketType type)
{
	int kind;
	switch (type) {
		case LOSKI_DGRMSOCKET:
			kind = SOCK_DGRAM;
			break;
		default:
			kind = SOCK_STREAM;
			break;
	}
	*sock = socket(AF_INET, kind, 0);
	if (*sock < 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_getsocketid(loski_NetState *state, loski_Socket *sock)
{
	return *sock;
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

LOSKIDRV_API int loski_setsocketoption(loski_NetState *state,
                                       loski_Socket *sock,
                                       loski_SocketOption option,
                                       int value)
{
	int res;
	switch (option) {
		case LOSKI_SOCKOPT_BLOCKING: {
			res = fcntl(*sock, F_GETFL, 0);
			if (res >= 0) {
				res = value ? (res & (~(O_NONBLOCK))) : (res | O_NONBLOCK);
				res = fcntl(*sock, F_SETFL, res);
				if (res != -1) res = 0;
			}
		} break;
		case LOSKI_SOCKOPT_LINGER: {
			struct linger li;
			if (value > 0) {
				li.l_onoff = 1;
				li.l_linger = value;
			} else {
				li.l_onoff = 0;
				li.l_linger = 0;
			}
			res = setsockopt(*sock, SOL_SOCKET, SO_LINGER, &li, sizeof(li));
		} break;
		default:
			res = setsockopt(*sock, optinfo[option].level, optinfo[option].name,
			                 &value, sizeof(value));
			break;
	}
	if (res != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_getsocketoption(loski_NetState *state,
                                       loski_Socket *sock,
                                       loski_SocketOption option,
                                       int *value)
{
	int res;
	switch (option) {
		case LOSKI_SOCKOPT_BLOCKING: {
			res = fcntl(*sock, F_GETFL, 0);
			if (res >= 0) {
				*value = !(res & O_NONBLOCK);
				res = 0;
			}
		} break;
		case LOSKI_SOCKOPT_LINGER: {
			struct linger li;
			socklen_t sz = sizeof(li);
			res = getsockopt(*sock, SOL_SOCKET, SO_LINGER, &li, &sz);
			if (res == 0) *value = li.l_onoff ? li.l_linger : 0;
		} break;
		default: {
			socklen_t sz = sizeof(*value);
			res = getsockopt(*sock, optinfo[option].level, optinfo[option].name,
			                 value, &sz);
		} break;
	}
	if (res != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_bindsocket(loski_NetState *state,
                                  loski_Socket *sock,
                                  const loski_Address *address)
{
	if (bind(*sock, address, sizeof(loski_Address)) != 0) return errno; 
	return 0;
}

LOSKIDRV_API int loski_socketaddress(loski_NetState *state,
                                     loski_Socket *sock,
                                     loski_Address *address,
                                     loski_SocketSite site)
{
	int res;
	socklen_t len = sizeof(loski_Address);
	if (site == LOSKI_LOCALSITE) res = getsockname(*sock, address, &len);
	else                         res = getpeername(*sock, address, &len);
	if (res != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_connectsocket(loski_NetState *state,
                                     loski_Socket *sock,
                                     const loski_Address *address)
{
	if (connect(*sock, address, sizeof(loski_Address)) != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_sendtosocket(loski_NetState *state,
                                    loski_Socket *sock,
                                    const char *data,
                                    size_t size,
                                    size_t *bytes,
                                    const loski_Address *address)
{
	ssize_t res;
	if (address) {
		res = sendto(*sock, data, size, 0, address, sizeof(loski_Address));
	} else {
		res = send(*sock, data, size, 0);
	}
	if (res < 0) return errno;
	*bytes = (size_t)res;
	return 0;
}

LOSKIDRV_API int loski_recvfromsocket(loski_NetState *state,
                                      loski_Socket *sock,
                                      char *buffer,
                                      size_t size,
                                      size_t *bytes,
                                      loski_Address *address)
{
	ssize_t res;
	if (address) {
		socklen_t len = sizeof(loski_Address);
		memset(address, 0, len);
		res = recvfrom(*sock, buffer, size, 0, address, &len);
	} else {
		res = recv(*sock, buffer, size, 0);
	}
	if (res == 0) return ECONNABORTED;
	if (res < 0) return errno;
	*bytes = (size_t)res;
	return 0;
}

LOSKIDRV_API int loski_shutdownsocket(loski_NetState *state,
                                      loski_Socket *sock,
                                      loski_SocketSite site)
{
	int how;
	switch (site) {
		case LOSKI_LOCALSITE:
			how = SHUT_WR;
			break;
		case LOSKI_REMOTESITE:
			how = SHUT_RD;
			break;
		case LOSKI_BOTHSITES:
			how = SHUT_RDWR;
			break;
		default:
			return 0;
	}
	if (shutdown(*sock, how) != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_acceptsocket(loski_NetState *state,
                                    loski_Socket *sock,
                                    loski_Socket *accepted,
                                    loski_Address *address)
{
	socklen_t len = 0;
	if (address) {
		len = sizeof(loski_Address);
		memset(address, 0, len);
	}
	*accepted = accept(*sock, address, &len);
	if (*accepted < 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_listensocket(loski_NetState *state,
                                    loski_Socket *sock,
                                    int backlog)
{
	if (listen(*sock, backlog) != 0) return errno;
	return 0;
}

LOSKIDRV_API int loski_closesocket(loski_NetState *state,
                                   loski_Socket *sock)
{
	if (close(*sock) != 0) return errno;
	return 0;
}
