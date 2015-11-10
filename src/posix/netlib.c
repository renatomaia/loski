#include "netlib.h"


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


#include <errno.h>
#include <signal.h> /* sigpipe handling */

LOSKIDRV_API int loskiN_initdrv (loski_NetDriver *drv)
{
	/* instals a handler to ignore sigpipe or it will crash us */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return LOSKI_ERRUNEXPECTED;
	return 0;
}

LOSKIDRV_API int loskiN_freedrv (loski_NetDriver *drv)
{
	if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) return LOSKI_ERRUNEXPECTED;
	return 0;
}


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


#include <string.h>

/* TODO: ASSERT(LOSKI_ADDRSIZE_IPV4 == sizeof(in_addr_t)) */

#define MAXLITERAL	16

LOSKIDRV_API int loskiN_initaddr (loski_NetDriver *drv,
                                  loski_Address *address,
                                  loski_AddressType type)
{
	memset(address, 0, sizeof(loski_Address));
	address->ss_family = type;
	return 0;
}

LOSKIDRV_API int loskiN_setaddrport (loski_NetDriver *drv,
                                    loski_Address *address,
                                    loski_AddressPort port)
{
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			addr->sin_port = htons(port);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			addr->sin6_port = htons(port);
		} break;
	}
	return LOSKI_ERRUNSUPPORTED;
}

LOSKIDRV_API int loskiN_setaddrbytes (loski_NetDriver *drv,
                                      loski_Address *address,
                                      const char *data)
{
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			memcpy(&(addr->sin_addr.s_addr), data, LOSKI_ADDRSIZE_IPV4);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			memcpy(&addr->sin6_addr, data, LOSKI_ADDRSIZE_IPV6);
		} break;
	}
	return LOSKI_ERRUNSUPPORTED;
}

LOSKIDRV_API int loskiN_setaddrliteral (loski_NetDriver *drv,
                                        loski_Address *address,
                                        const char *data)
{
	void *bytes;
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			bytes = (void *)&(addr->sin_addr.s_addr);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			bytes = (void *)&addr->sin6_addr;
		} break;
		default: return LOSKI_ERRUNSUPPORTED;
	}
	if (inet_pton(address->ss_family, data, bytes) == 1) return 0;
	return LOSKI_ERRUNEXPECTED;
}

LOSKIDRV_API loski_AddressType loskiN_getaddrtype (loski_NetDriver *drv,
                                                   loski_Address *address)
{
	return address->ss_family;
}

LOSKIDRV_API loski_AddressPort loskiN_getaddrport (loski_NetDriver *drv,
                                                   loski_Address *address)
{
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			return ntohs(addr->sin_port);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			return ntohs(addr->sin6_port);
		} break;
	}
	return 0;
}

LOSKIDRV_API const char *loskiN_getaddrbytes (loski_NetDriver *drv,
                                              loski_Address *address,
                                              size_t *sz)
{
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			if (sz) *sz = LOSKI_ADDRSIZE_IPV4;
			return (const void *)&(addr->sin_addr.s_addr);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			if (sz) *sz = LOSKI_ADDRSIZE_IPV6;
			return (const void *)&addr->sin6_addr;
		} break;
	}
	if (sz) *sz = 0;
	return NULL;
}

LOSKIDRV_API const char *loskiN_getaddrliteral (loski_NetDriver *drv,
                                                loski_Address *address,
                                                char *data)
{
	const void *bytes;
	switch (address->ss_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			bytes = (const void *)&(addr->sin_addr.s_addr);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			bytes = (const void *)&addr->sin6_addr;
		} break;
		default: return NULL;
	}
	return inet_ntop(address->ss_family, bytes, data, LOSKI_ADDRMAXLITERAL);
}

#define toaddr(A)	((struct sockaddr*)A)


/*****************************************************************************
 * Sockets *******************************************************************
 *****************************************************************************/


#include <unistd.h>
#include <netinet/tcp.h> /* TCP options (nagle algorithm disable) */
#include <fcntl.h> /* fnctnl function and associated constants */

LOSKIDRV_API int loskiN_initsock (loski_NetDriver *drv,
                                  loski_Socket *sock,
                                  loski_SocketType type,
                                  loski_AddressType domain)
{
	int kind;
	switch (type) {
		case LOSKI_LSTNSOCKET:
		case LOSKI_CONNSOCKET: kind = SOCK_STREAM; break;
		case LOSKI_DGRMSOCKET: kind = SOCK_DGRAM; break;
		default: return LOSKI_ERRUNSUPPORTED;
	}
	if (domain != LOSKI_ADDRTYPE_IPV4) return LOSKI_ERRUNSUPPORTED;
	*sock = socket(domain, kind, 0);
	if (*sock >= 0) return 0;
	switch (errno) {
		case EACCES: return LOSKI_ERRDENIED;
		case EMFILE:
		case ENFILE: return LOSKI_ERRNORESOURCES;
		case ENOBUFS:
		case ENOMEM: return LOSKI_ERRNOMEMORY;
		case EAFNOSUPPORT:
		case EPROTONOSUPPORT:
		case EPROTOTYPE: return LOSKI_ERRUNSUPPORTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_getsockid (loski_NetDriver *drv, loski_Socket *sock)
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

LOSKIDRV_API int loskiN_setsockopt (loski_NetDriver *drv,
                                    loski_Socket *sock,
                                    loski_SocketOption option,
                                    int value)
{
	int err;
	switch (option) {
		case LOSKI_SOCKOPT_BLOCKING: {
			err = fcntl(*sock, F_GETFL, 0);
			if (err >= 0) {
				value = value ? (err & (~(O_NONBLOCK))) : (err | O_NONBLOCK);
				if (value != err) err = fcntl(*sock, F_SETFL, value);
				if (err != -1) err = 0;
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
			err = setsockopt(*sock, SOL_SOCKET, SO_LINGER, &li, sizeof(li));
		} break;
		default:
			err = setsockopt(*sock, optinfo[option].level, optinfo[option].name,
			                 &value, sizeof(value));
			break;
	}
	if (err == 0) return 0;
	switch (errno) {
		case EISCONN: return LOSKI_ERRINUSE;
		case EDOM: return LOSKI_ERRTOOMUCH;
		case EINVAL: return LOSKI_ERRINVALID;
		case ENOPROTOOPT: return LOSKI_ERRUNSUPPORTED;
		case ENOTSOCK:
		case EBADF: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_getsockopt (loski_NetDriver *drv,
                                    loski_Socket *sock,
                                    loski_SocketOption option,
                                    int *value)
{
	int err;
	switch (option) {
		case LOSKI_SOCKOPT_BLOCKING: {
			err = fcntl(*sock, F_GETFL, 0);
			if (err >= 0) {
				*value = !(err & O_NONBLOCK);
				err = 0;
			}
		} break;
		case LOSKI_SOCKOPT_LINGER: {
			struct linger li;
			socklen_t sz = sizeof(li);
			err = getsockopt(*sock, SOL_SOCKET, SO_LINGER, &li, &sz);
			if (err == 0) *value = li.l_onoff ? li.l_linger : 0;
		} break;
		default: {
			socklen_t sz = sizeof(*value);
			err = getsockopt(*sock, optinfo[option].level, optinfo[option].name,
			                 value, &sz);
		} break;
	}
	if (err == 0) return 0;
	switch (errno) {
		case EACCES: return LOSKI_ERRDENIED;
		case ENOBUFS: return LOSKI_ERRNOMEMORY;
		case EINVAL: return LOSKI_ERRINVALID;
		case ENOPROTOOPT: return LOSKI_ERRUNSUPPORTED;
		case ENOTSOCK:
		case EBADF: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_bindsock (loski_NetDriver *drv,
                                  loski_Socket *sock,
                                  const loski_Address *address)
{
	if (bind(*sock, toaddr(address), sizeof(loski_Address)) == 0) return 0; 
	switch (errno) {
		case EISCONN: return LOSKI_ERRINUSE;
		case EADDRINUSE: return LOSKI_ERRUNAVAILABLE;
		case EADDRNOTAVAIL: return LOSKI_ERRUNREACHABLE;
		case EACCES: return LOSKI_ERRDENIED;
		case ENOBUFS: return LOSKI_ERRNOMEMORY;
		case EOPNOTSUPP: return LOSKI_ERRUNSUPPORTED;
		case EAFNOSUPPORT:
		case EINVAL: return LOSKI_ERRINVALID;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
#if 0
		/* only for AF_UNIX sockets */
		case ENOENT: return LOSKI_ERRNOTFOUND;
		case ENOTDIR: return LOSKI_ERRUNAVAILABLE;
		case EROFS: return LOSKI_ERRDENIED;
		case EISDIR:
		case EIO: return LOSKI_ERRSYSTEMFAIL;
		case ELOOP:
		case ENAMETOOLONG: return LOSKI_ERRTOOMUCH;
		case EDESTADDRREQ: return LOSKI_ERRUNEXPECTED;
#endif
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_getsockaddr (loski_NetDriver *drv,
                                     loski_Socket *sock,
                                     loski_Address *address,
                                     int peer)
{
	int err;
	socklen_t len = sizeof(loski_Address);
	if (peer) err = getpeername(*sock, toaddr(address), &len);
	else      err = getsockname(*sock, toaddr(address), &len);
	if (err == 0) return 0;
	switch (errno) {
		case ENOBUFS: return LOSKI_ERRNOMEMORY;
		case EINVAL: return LOSKI_ERRINVALID;
		case EOPNOTSUPP: return LOSKI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
		case ENOTCONN: if (peer) return LOSKI_ERRCLOSED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_connectsock (loski_NetDriver *drv,
                                     loski_Socket *sock,
                                     const loski_Address *address)
{
	if (connect(*sock, toaddr(address), sizeof(loski_Address)) == 0) return 0;
	switch (errno) {
		case EALREADY:
		case EINPROGRESS:
		case EINTR: return LOSKI_ERRUNFULFILLED;
		case ETIMEDOUT: return LOSKI_ERRTIMEOUT;
		case EADDRINUSE: return LOSKI_ERRINUSE;
		case EADDRNOTAVAIL: return LOSKI_ERRUNAVAILABLE;
		case ECONNRESET: return LOSKI_ERRCLOSED;
		case EHOSTUNREACH:
		case ENETUNREACH: return LOSKI_ERRUNREACHABLE;
		case ECONNREFUSED: return LOSKI_ERRREFUSED;
		case ENETDOWN: return LOSKI_ERRSYSTEMDOWN;
		case ENOBUFS: return LOSKI_ERRNOMEMORY;
		case EISCONN:
		case EPROTOTYPE:
		case EAFNOSUPPORT:
		case EOPNOTSUPP: return LOSKI_ERRINVALID;
		case EINVAL: return LOSKI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
#if 0
		/* only for AF_UNIX sockets */
		case EACCES: return LOSKI_ERRDENIED;
		case ENOENT: return LOSKI_ERRNOTFOUND;
		case ENOTDIR: return LOSKI_ERRUNAVAILABLE;
		case EIO: return LOSKI_ERRSYSTEMFAIL;
		case ELOOP:
		case ENAMETOOLONG: return LOSKI_ERRTOOMUCH;
#endif
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_sendtosock (loski_NetDriver *drv,
                                    loski_Socket *sock,
                                    const char *data,
                                    size_t size,
                                    size_t *bytes,
                                    const loski_Address *address)
{
	ssize_t err;
	if (!address) err = send(*sock, data, size, 0);
	else          err = sendto(*sock, data, size, 0,
	                           toaddr(address),
	                           sizeof(loski_Address));
	if (err >= 0) {
		*bytes = (size_t)err;
		return 0;
	}
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSKI_ERRUNFULFILLED;
		case EMSGSIZE: return LOSKI_ERRTOOMUCH;
		case ENOTCONN: return LOSKI_ERRCLOSED;
		case EPIPE:
		case ECONNRESET: return LOSKI_ERRABORTED;
		case EACCES: return LOSKI_ERRDENIED;
		case ENETUNREACH: return LOSKI_ERRUNREACHABLE;
		case ENOBUFS: return LOSKI_ERRNOMEMORY;
		case ENETDOWN: return LOSKI_ERRSYSTEMDOWN;
		case EIO: return LOSKI_ERRSYSTEMFAIL;
		case EDESTADDRREQ: return LOSKI_ERRINVALID;
		case EOPNOTSUPP: return LOSKI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_recvfromsock (loski_NetDriver *drv,
                                      loski_Socket *sock,
                                      loski_SocketRecvFlag flags,
                                      char *buffer,
                                      size_t size,
                                      size_t *bytes,
                                      loski_Address *address)
{
	ssize_t err;
	if (size == 0) return LOSKI_ERRINVALID;
	if (address) {
		socklen_t len = sizeof(loski_Address);
		memset(address, 0, len);
		err = recvfrom(*sock, buffer, size, flags, toaddr(address), &len);
	} else {
		err = recv(*sock, buffer, size, flags);
	}
	if (err > 0) {
		*bytes = (size_t)err;
		return 0;
	}
	if (err == 0) return LOSKI_ERRCLOSED;
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSKI_ERRUNFULFILLED;
		case ETIMEDOUT: return LOSKI_ERRTIMEOUT;
		case ENOTCONN: return LOSKI_ERRCLOSED;
		case ECONNRESET: return LOSKI_ERRABORTED;
		case ENOMEM: return LOSKI_ERRNOMEMORY;
		case ENOBUFS: return LOSKI_ERRNORESOURCES;
		case ENETDOWN: return LOSKI_ERRSYSTEMDOWN;
		case EIO: return LOSKI_ERRSYSTEMFAIL;
		case EOPNOTSUPP: return LOSKI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_shutdownsock (loski_NetDriver *drv,
                                      loski_Socket *sock,
                                      loski_SocketWay ways)
{
	if (shutdown(*sock, ways) == 0) return 0;
	switch (errno) {
		case ENOTCONN: return LOSKI_ERRCLOSED;
		case ENOBUFS: return LOSKI_ERRNORESOURCES;
		case EINVAL:
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_acceptsock (loski_NetDriver *drv,
                                    loski_Socket *sock,
                                    loski_Socket *accepted,
                                    loski_Address *address)
{
	socklen_t len = 0;
	if (address) {
		len = sizeof(loski_Address);
		memset(address, 0, len);
	}
	*accepted = accept(*sock, toaddr(address), &len);
	if (*accepted >= 0) return 0;
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSKI_ERRUNFULFILLED;
		case EMFILE:
		case ENFILE: return LOSKI_ERRNORESOURCES;
		case ECONNABORTED: return LOSKI_ERRABORTED;
		case ENOBUFS: return LOSKI_ERRNORESOURCES;
		case ENOMEM: return LOSKI_ERRNOMEMORY;
		case EINVAL: return LOSKI_ERRINVALID;
		case EOPNOTSUPP: return LOSKI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_listensock (loski_NetDriver *drv,
                                    loski_Socket *sock,
                                    int backlog)
{
	if (listen(*sock, backlog) == 0) return 0;
	switch (errno) {
		case EACCES: return LOSKI_ERRDENIED;
		case ENOBUFS: return LOSKI_ERRNORESOURCES;
		case EDESTADDRREQ: return LOSKI_ERRINVALID;
		case EINVAL:
		case EOPNOTSUPP:
		case EBADF:
		case ENOTSOCK: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_closesock (loski_NetDriver *drv,
                                   loski_Socket *sock)
{
	if (close(*sock) == 0) return 0;
	switch (errno) {
		case EINTR: return LOSKI_ERRUNFULFILLED;
		case EIO: return LOSKI_ERRSYSTEMFAIL;
		case EBADF: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}


/*****************************************************************************
 * Names *********************************************************************
 *****************************************************************************/


#include <netdb.h> /* gethostbyname and gethostbyaddr functions */

#define DGRMFLAG	1
#define LSTNFLAG	2

LOSKIDRV_API void loskiN_freeaddrfound (loski_NetDriver *drv,
                                        loski_AddressFound *found)
{
	if (found->results) {
		freeaddrinfo(found->results);
		found->results = NULL;
		found->next = NULL;
	}
}

static int hasnext (loski_AddressFound *found)
{
	while (found->next) {
		switch (found->next->ai_socktype) {
			case SOCK_DGRAM: found->nexttype |= DGRMFLAG; return 1;
			case SOCK_STREAM: found->nexttype &= ~DGRMFLAG; return 1;
		}
		found->next = found->next->ai_next;
	}
	loskiN_freeaddrfound(NULL, found);
	return 0;
}

LOSKIDRV_API int loskiN_resolveaddr (loski_NetDriver *drv,
                                     loski_AddressFound *found,
                                     loski_AddressFindFlag flags,
                                     const char *nodename,
                                     const char *servname)
{
	int err;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	found->nexttype = 0;
	if (flags & (LOSKI_ADDRFIND_IPV4|LOSKI_ADDRFIND_IPV6)) {
		if (flags & LOSKI_ADDRFIND_IPV4) hints.ai_family = AF_INET;
		if (flags & LOSKI_ADDRFIND_IPV6)
			hints.ai_family = (hints.ai_family==AF_INET) ? AF_UNSPEC : AF_INET6;
	} else {
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags |= AI_ADDRCONFIG;
	}
	if (flags & LOSKI_ADDRFIND_DGRM) hints.ai_socktype = SOCK_DGRAM;
	if (flags & LOSKI_ADDRFIND_STRM)
		hints.ai_socktype = (hints.ai_socktype==SOCK_DGRAM) ? 0 : SOCK_STREAM;
	if (flags & LOSKI_ADDRFIND_LSTN) {
		found->nexttype |= LSTNFLAG;
		hints.ai_flags |= AI_PASSIVE;
	}
	if (flags & LOSKI_ADDRFIND_MAP4) {
		hints.ai_flags |= AI_V4MAPPED;
		if (hints.ai_family == AF_INET) return LOSKI_ERRINVALID;
		if (hints.ai_family == AF_UNSPEC) hints.ai_flags |= AI_ALL;
		hints.ai_family = AF_INET6;
	}
	err = getaddrinfo(nodename, servname, &hints, &found->results);
	if (!err) {
		found->next = found->results;
		if (hasnext(found)) return 0;
		err = EAI_AGAIN;
	}
	switch (err) {
		case EAI_AGAIN:
		case EAI_SERVICE:
		case EAI_NONAME: return LOSKI_ERRNOTFOUND;
		case EAI_MEMORY: return LOSKI_ERRNOMEMORY;
		case EAI_FAIL:
		case EAI_SYSTEM: return LOSKI_ERRSYSTEMFAIL;
		case EAI_FAMILY:
		case EAI_SOCKTYPE:
		case EAI_BADFLAGS: return LOSKI_ERRUNSUPPORTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiN_getaddrfound (loski_NetDriver *drv,
                                      loski_AddressFound *found,
                                      loski_Address *address,
                                      loski_SocketType *type)
{
	memcpy(address, found->next->ai_addr, found->next->ai_addrlen);
	*type = (found->nexttype & DGRMFLAG ? LOSKI_DGRMSOCKET :
	        (found->nexttype & LSTNFLAG ? LOSKI_LSTNSOCKET : LOSKI_CONNSOCKET));
	found->next = found->next->ai_next;
	return hasnext(found);
}

