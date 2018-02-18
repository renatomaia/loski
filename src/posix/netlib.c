#include "luaosi/netlib.h"


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


#include <assert.h>
#include <errno.h>
#include <signal.h> /* sigpipe handling */

LOSIDRV_API losi_ErrorCode losiN_initdrv (losi_NetDriver *drv)
{
	/* installs a handler to ignore sigpipe or it will crash us */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return LOSI_ERRUNEXPECTED;
	return LOSI_ERRNONE;
}

LOSIDRV_API void losiN_freedrv (losi_NetDriver *drv)
{
	signal(SIGPIPE, SIG_DFL);
}


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


#include <string.h>

/* TODO: ASSERT(LOSI_ADDRBINSZ_IPV4 == sizeof(in_addr_t)) */

static size_t addrtypesz (const losi_AddressType type)
{
	switch (type) {
		case LOSI_ADDRTYPE_IPV4: return LOSI_ADDRSIZE_IPV4;
		case LOSI_ADDRTYPE_IPV6: return LOSI_ADDRSIZE_IPV6;
		case LOSI_ADDRTYPE_FILE: return LOSI_ADDRSIZE_FILE;
	}
	return sizeof(struct sockaddr_storage);
}

LOSIDRV_API losi_ErrorCode losiN_initaddr (losi_NetDriver *drv,
                                           losi_Address *address,
                                           losi_AddressType type)
{
	memset(address, 0, addrtypesz(type));
	address->sa_family = type;
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiN_setaddrport (losi_NetDriver *drv,
                                              losi_Address *address,
                                              losi_AddressPort port)
{
	switch (address->sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			addr->sin_port = htons(port);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			addr->sin6_port = htons(port);
		} break;
		default: return LOSI_ERRUNSUPPORTED;
	}
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiN_setaddrbytes (losi_NetDriver *drv,
                                               losi_Address *address,
                                               const char *data)
{
	switch (address->sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			memcpy(&(addr->sin_addr.s_addr), data, LOSI_ADDRBINSZ_IPV4);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			memcpy(&addr->sin6_addr, data, LOSI_ADDRBINSZ_IPV6);
		} break;
		case AF_UNIX: {
			struct sockaddr_un *addr = (struct sockaddr_un *)address;
			memcpy(&addr->sun_path, data, LOSI_ADDRBINSZ_FILE);
			addr->sun_path[LOSI_ADDRBINSZ_FILE] = '\0';
		} break;
		default: return LOSI_ERRUNSUPPORTED;
	}
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiN_setaddrliteral (losi_NetDriver *drv,
                                                 losi_Address *address,
                                                 const char *data)
{
	if (address->sa_family == AF_UNIX) {
		struct sockaddr_un *addr = (struct sockaddr_un *)address;
		char *e = stpncpy(addr->sun_path, data, LOSI_ADDRBINSZ_FILE);
		if (e == (addr->sun_path + LOSI_ADDRBINSZ_FILE)) {
			if (data[LOSI_ADDRBINSZ_FILE]) return LOSI_ERRTOOMUCH;
			*e = '\0';
		}
		return LOSI_ERRNONE;
	} else {
		int err;
		void *bytes;
		switch (address->sa_family) {
			case AF_INET: {
				struct sockaddr_in *addr = (struct sockaddr_in *)address;
				bytes = (void *)&(addr->sin_addr.s_addr);
			} break;
			case AF_INET6: {
				struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
				bytes = (void *)&addr->sin6_addr;
			} break;
			default: return LOSI_ERRUNSUPPORTED;
		}
		err = inet_pton(address->sa_family, data, bytes);
		if (err == 1) return LOSI_ERRNONE;
		if (err == 0) return LOSI_ERRINVALID;
		if (errno == EAFNOSUPPORT) return LOSI_ERRUNSUPPORTED;
		return LOSI_ERRUNSPECIFIED;
	}
}

LOSIDRV_API losi_AddressType losiN_getaddrtype (losi_NetDriver *drv,
                                                losi_Address *address)
{
	return address->sa_family;
}

LOSIDRV_API losi_AddressPort losiN_getaddrport (losi_NetDriver *drv,
                                                losi_Address *address)
{
	switch (address->sa_family) {
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

LOSIDRV_API const char *losiN_getaddrbytes (losi_NetDriver *drv,
                                            losi_Address *address,
                                            size_t *sz)
{
	switch (address->sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			if (sz) *sz = LOSI_ADDRBINSZ_IPV4;
			return (const void *)&(addr->sin_addr.s_addr);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			if (sz) *sz = LOSI_ADDRBINSZ_IPV6;
			return (const void *)&addr->sin6_addr;
		} break;
		case AF_UNIX: {
			struct sockaddr_un *addr = (struct sockaddr_un *)address;
			if (sz) *sz = LOSI_ADDRBINSZ_FILE;
			return addr->sun_path;
		} break;
	}
	if (sz) *sz = 0;
	return NULL;
}

LOSIDRV_API const char *losiN_getaddrliteral (losi_NetDriver *drv,
                                              losi_Address *address,
                                              char *data)
{
	const void *bytes;
	switch (address->sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			bytes = (const void *)&(addr->sin_addr.s_addr);
		} break;
		case AF_INET6: {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			bytes = (const void *)&addr->sin6_addr;
		} break;
		case AF_UNIX: {
			struct sockaddr_un *addr = (struct sockaddr_un *)address;
			return addr->sun_path;
		} break;
		default: return NULL;
	}
	return inet_ntop(address->sa_family, bytes, data, LOSI_ADDRMAXLITERAL);
}

#define toaddr(A) ((struct sockaddr *)A)
#define addrsz(A) (addrtypesz(A->sa_family))

/*****************************************************************************
 * Sockets *******************************************************************
 *****************************************************************************/


#include "luaosi/lnetlib.h"
#include "luaosi/fdlib.h"

#include <unistd.h>
#include <netinet/tcp.h> /* TCP options (nagle algorithm disable) */

LOSIDRV_API losi_ErrorCode losiN_initsock (losi_NetDriver *drv,
                                           losi_Socket *sock,
                                           losi_SocketType type,
                                           losi_AddressType domain)
{
	int kind;
	switch (type) {
		case LOSI_SOCKTYPE_LSTN:
		case LOSI_SOCKTYPE_STRM: kind = SOCK_STREAM; break;
		case LOSI_SOCKTYPE_DGRM: kind = SOCK_DGRAM; break;
		default: return LOSI_ERRUNSUPPORTED;
	}
	sock->domain = domain;
	sock->fd = socket(domain, kind, 0);
	if (sock->fd >= 0) return LOSI_ERRNONE;
	switch (errno) {
		case EACCES: return LOSI_ERRDENIED;
		case EMFILE:
		case ENFILE: return LOSI_ERRNORESOURCES;
		case ENOBUFS:
		case ENOMEM: return LOSI_ERRNOMEMORY;
		case EAFNOSUPPORT:
		case EPROTONOSUPPORT:
		case EPROTOTYPE: return LOSI_ERRUNSUPPORTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_IntUniqueId losiN_getsockid (losi_NetDriver *drv,
                                              losi_Socket *sock)
{
	return sock->fd;
}

LOSIDRV_API losi_AddressType losiN_getsockdomain (losi_NetDriver *drv,
                                                  losi_Socket *sock)
{
	return sock->domain;
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

LOSIDRV_API losi_ErrorCode losiN_setsockopt (losi_NetDriver *drv,
                                             losi_Socket *sock,
                                             losi_SocketOption option,
                                             int value)
{
	int err;
	switch (option) {
		case LOSI_SOCKOPT_BLOCKING: return losiFD_setnonblock(sock->fd, value);
		case LOSI_SOCKOPT_LINGER: {
			struct linger li;
			if (value >= 0) {
				li.l_onoff = 1;
				li.l_linger = value;
			} else {
				li.l_onoff = 0;
				li.l_linger = 0;
			}
			err = setsockopt(sock->fd, SOL_SOCKET, SO_LINGER, &li, sizeof(li));
		} break;
		default:
			err = setsockopt(sock->fd, optinfo[option].level, optinfo[option].name,
			                 &value, sizeof(value));
			break;
	}
	if (err == 0) return LOSI_ERRNONE;
	switch (errno) {
		case EISCONN: return LOSI_ERRINUSE;
		case EDOM: return LOSI_ERRTOOMUCH;
		case EINVAL: return LOSI_ERRINVALID;
		case ENOPROTOOPT: return LOSI_ERRUNSUPPORTED;
		case ENOTSOCK:
		case EBADF: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_getsockopt (losi_NetDriver *drv,
                                             losi_Socket *sock,
                                             losi_SocketOption option,
                                             int *value)
{
	int err;
	switch (option) {
		case LOSI_SOCKOPT_BLOCKING: return losiFD_getnonblock(sock->fd, value);
		case LOSI_SOCKOPT_LINGER: {
			struct linger li;
			socklen_t sz = sizeof(li);
			err = getsockopt(sock->fd, SOL_SOCKET, SO_LINGER, &li, &sz);
			if (err == 0) *value = li.l_onoff ? li.l_linger : -1;
		} break;
		default: {
			socklen_t sz = sizeof(*value);
			err = getsockopt(sock->fd, optinfo[option].level, optinfo[option].name,
			                 value, &sz);
		} break;
	}
	if (err == 0) return LOSI_ERRNONE;
	switch (errno) {
		case EACCES: return LOSI_ERRDENIED;
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case EINVAL: return LOSI_ERRINVALID;
		case ENOPROTOOPT: return LOSI_ERRUNSUPPORTED;
		case ENOTSOCK:
		case EBADF: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_bindsock (losi_NetDriver *drv,
                                           losi_Socket *sock,
                                           const losi_Address *address)
{
	if (bind(sock->fd, toaddr(address), addrsz(address)) == 0)
		return LOSI_ERRNONE; 
	switch (errno) {
		case EISCONN: return LOSI_ERRINUSE;
		case EADDRINUSE: return LOSI_ERRUNAVAILABLE;
		case EADDRNOTAVAIL: return LOSI_ERRUNREACHABLE;
		case EACCES: return LOSI_ERRDENIED;
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case EOPNOTSUPP: return LOSI_ERRUNSUPPORTED;
		case EAFNOSUPPORT:
		case EINVAL: return LOSI_ERRINVALID;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
		/* only for AF_UNIX sockets */
		case ENOENT: return LOSI_ERRNOTFOUND;
		case ENOTDIR: return LOSI_ERRUNAVAILABLE;
		case EROFS: return LOSI_ERRDENIED;
		case EISDIR:
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case ELOOP:
		case ENAMETOOLONG: return LOSI_ERRTOOMUCH;
		case EDESTADDRREQ: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_getsockaddr (losi_NetDriver *drv,
                                              losi_Socket *sock,
                                              losi_Address *address,
                                              int peer)
{
	int err;
	socklen_t len = addrsz(address);
	if (peer) err = getpeername(sock->fd, toaddr(address), &len);
	else      err = getsockname(sock->fd, toaddr(address), &len);
	if (err == 0) return LOSI_ERRNONE;
	switch (errno) {
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case EINVAL: return LOSI_ERRINVALID;
		case EOPNOTSUPP: return LOSI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
		case ENOTCONN: if (peer) return LOSI_ERRCLOSED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_connectsock (losi_NetDriver *drv,
                                              losi_Socket *sock,
                                              const losi_Address *address)
{
	if (connect(sock->fd, toaddr(address), addrsz(address)) == 0)
		return LOSI_ERRNONE;
	switch (errno) {
		case EALREADY:
		case EINPROGRESS:
		case EINTR: return LOSI_ERRUNFULFILLED;
		case ETIMEDOUT: return LOSI_ERRTIMEOUT;
		case EISCONN:
		case EADDRINUSE: return LOSI_ERRINUSE;
		case EADDRNOTAVAIL: return LOSI_ERRUNAVAILABLE;
		case ECONNRESET: return LOSI_ERRCLOSED;
		case EHOSTUNREACH:
		case ENETUNREACH: return LOSI_ERRUNREACHABLE;
		case ECONNREFUSED: return LOSI_ERRREFUSED;
		case ENETDOWN: return LOSI_ERRSYSTEMDOWN;
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case EPROTOTYPE:
		case EAFNOSUPPORT:
		case EOPNOTSUPP: return LOSI_ERRINVALID;
		case EINVAL: return LOSI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
		/* only for AF_UNIX sockets */
		case EACCES: return LOSI_ERRDENIED;
		case ENOENT: return LOSI_ERRNOTFOUND;
		case ENOTDIR: return LOSI_ERRUNAVAILABLE;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case ELOOP:
		case ENAMETOOLONG: return LOSI_ERRTOOMUCH;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_sendtosock (losi_NetDriver *drv,
                                             losi_Socket *sock,
                                             const char *data,
                                             size_t size,
                                             size_t *bytes,
                                             const losi_Address *address)
{
	ssize_t err;
	if (!address) err = send(sock->fd, data, size, 0);
	else          err = sendto(sock->fd, data, size, 0, toaddr(address),
	                                                    addrsz(address));
	if (err >= 0) {
		*bytes = (size_t)err;
		return LOSI_ERRNONE;
	}
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSI_ERRUNFULFILLED;
		case EMSGSIZE: return LOSI_ERRTOOMUCH;
		case ENOTCONN: return LOSI_ERRCLOSED;
		case EPIPE:
		case ECONNRESET: return LOSI_ERRABORTED;
		case EACCES: return LOSI_ERRDENIED;
		case ENETUNREACH: return LOSI_ERRUNREACHABLE;
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case ENETDOWN: return LOSI_ERRSYSTEMDOWN;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case EDESTADDRREQ: return LOSI_ERRINVALID;
		case EOPNOTSUPP: return LOSI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_recvfromsock (losi_NetDriver *drv,
                                               losi_Socket *sock,
                                               losi_SocketRecvFlag flags,
                                               char *buffer,
                                               size_t size,
                                               size_t *bytes,
                                               losi_Address *address)
{
	ssize_t err;
	if (size == 0) return LOSI_ERRINVALID;
	if (address) {
		socklen_t len = addrsz(address);
		err = recvfrom(sock->fd, buffer, size, flags, toaddr(address), &len);
	} else {
		err = recv(sock->fd, buffer, size, flags);
	}
	if (err > 0) {
		*bytes = (size_t)err;
		return LOSI_ERRNONE;
	}
	if (err == 0) return LOSI_ERRCLOSED;
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSI_ERRUNFULFILLED;
		case ETIMEDOUT: return LOSI_ERRTIMEOUT;
		case ENOTCONN: return LOSI_ERRCLOSED;
		case ECONNRESET: return LOSI_ERRABORTED;
		case ENOMEM: return LOSI_ERRNOMEMORY;
		case ENOBUFS: return LOSI_ERRNORESOURCES;
		case ENETDOWN: return LOSI_ERRSYSTEMDOWN;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case EOPNOTSUPP: return LOSI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_shutdownsock (losi_NetDriver *drv,
                                               losi_Socket *sock,
                                               losi_SocketWay ways)
{
	if (shutdown(sock->fd, ways) == 0) return LOSI_ERRNONE;
	switch (errno) {
		case ENOTCONN: return LOSI_ERRCLOSED;
		case ENOBUFS: return LOSI_ERRNORESOURCES;
		case EINVAL:
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_acceptsock (losi_NetDriver *drv,
                                             losi_Socket *sock,
                                             losi_Socket *accepted,
                                             losi_Address *address)
{
	socklen_t len = 0;
	if (address) {
		len = addrsz(address);
		memset(address, 0, len);
	}
	accepted->fd = accept(sock->fd, toaddr(address), &len);
	if (accepted->fd >= 0) return LOSI_ERRNONE;
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK:
		case EINTR: return LOSI_ERRUNFULFILLED;
		case ECONNABORTED: return LOSI_ERRABORTED;
		case EMFILE:
		case ENFILE:
		case ENOBUFS: return LOSI_ERRNORESOURCES;
		case ENOMEM: return LOSI_ERRNOMEMORY;
		case EINVAL: return LOSI_ERRINVALID;
		case EOPNOTSUPP: return LOSI_ERRUNSUPPORTED;
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_listensock (losi_NetDriver *drv,
                                             losi_Socket *sock,
                                             int backlog)
{
	if (listen(sock->fd, backlog) == 0) return LOSI_ERRNONE;
	switch (errno) {
		case EACCES: return LOSI_ERRDENIED;
		case ENOBUFS: return LOSI_ERRNORESOURCES;
		case EDESTADDRREQ: return LOSI_ERRINVALID;
		case EINVAL:
		case EOPNOTSUPP:
		case EBADF:
		case ENOTSOCK: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_closesock (losi_NetDriver *drv,
                                            losi_Socket *sock)
{
	if (close(sock->fd) == 0) return LOSI_ERRNONE;
	switch (errno) {
		case EINTR: return LOSI_ERRUNFULFILLED;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case EBADF: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_getsockevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags)
{
	LuaSocket *ls = (LuaSocket *)udata;
	if (ls->refs == 0) return LOSI_ERRCLOSED;
	if (newref) ++(ls->refs);
	*src = ls->socket.fd;
	return LOSI_ERRNONE;
}

LOSIDRV_API void losiN_freesockevtsrc (void *udata)
{
	LuaSocket *ls = (LuaSocket *)udata;
	--(ls->refs);
	assert(ls->refs > 0);
}


/*****************************************************************************
 * Names *********************************************************************
 *****************************************************************************/


#include <netdb.h> /* gethostbyname and gethostbyaddr functions */

LOSIDRV_API void losiN_freeaddrfound (losi_NetDriver *drv,
                                      losi_AddressFound *found)
{
	if (found->results) {
		freeaddrinfo(found->results);
		found->results = NULL;
		found->next = NULL;
	}
}

static int hasnext (losi_AddressFound *found)
{
	while (found->next) {
		switch (found->next->ai_socktype) {
			case SOCK_DGRAM:
			case SOCK_STREAM: return 1;
		}
		found->next = found->next->ai_next;
	}
	losiN_freeaddrfound(NULL, found);
	return 0;
}

static int getaddrerr (int err)
{
	switch (err) {
		case EAI_AGAIN:
		case EAI_SERVICE:
		case EAI_NONAME: return LOSI_ERRNOTFOUND;
		case EAI_MEMORY: return LOSI_ERRNOMEMORY;
		case EAI_FAIL:
		case EAI_SYSTEM: return LOSI_ERRSYSTEMFAIL;
		case EAI_FAMILY:
		case EAI_SOCKTYPE:
		case EAI_BADFLAGS: return LOSI_ERRUNSUPPORTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiN_resolveaddr (losi_NetDriver *drv,
                                              losi_AddressFound *found,
                                              losi_AddressFindFlag flags,
                                              const char *nodename,
                                              const char *servname)
{
	int err;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	found->passive = 0;
	if (flags & (LOSI_ADDRFIND_IPV4|LOSI_ADDRFIND_IPV6)) {
		if (flags & LOSI_ADDRFIND_IPV4) hints.ai_family = AF_INET;
		if (flags & LOSI_ADDRFIND_IPV6)
			hints.ai_family = (hints.ai_family==AF_INET) ? AF_UNSPEC : AF_INET6;
	} else {
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags |= AI_ADDRCONFIG;
	}
	if (flags & LOSI_ADDRFIND_DGRM) hints.ai_socktype = SOCK_DGRAM;
	if (flags & LOSI_ADDRFIND_STRM)
		hints.ai_socktype = (hints.ai_socktype==SOCK_DGRAM) ? 0 : SOCK_STREAM;
	if (flags & LOSI_ADDRFIND_LSTN) {
		found->passive = 1;
		hints.ai_flags |= AI_PASSIVE;
	}
	if (flags & LOSI_ADDRFIND_MAP4) {
		hints.ai_flags |= AI_V4MAPPED;
		if (hints.ai_family == AF_INET) return LOSI_ERRINVALID;
		if (hints.ai_family == AF_UNSPEC) hints.ai_flags |= AI_ALL;
		hints.ai_family = AF_INET6;
	}
	err = getaddrinfo(nodename, servname, &hints, &found->results);
	if (!err) {
		found->next = found->results;
		if (hasnext(found)) return LOSI_ERRNONE;
		err = EAI_AGAIN;
	}
	return getaddrerr(err);
}

LOSIDRV_API int losiN_getaddrtypefound (losi_NetDriver *drv,
                                        losi_AddressFound *found,
                                        losi_AddressType *type)
{
	if (found->next == NULL) return 0;
	*type = found->next->ai_family;
	return 1;
}

LOSIDRV_API int losiN_getaddrfound (losi_NetDriver *drv,
                                    losi_AddressFound *found,
                                    losi_Address *address,
                                    losi_SocketType *type)
{
	memcpy(address, found->next->ai_addr, found->next->ai_addrlen);
	*type = (found->next->ai_socktype == SOCK_DGRAM ? LOSI_SOCKTYPE_DGRM :
	        (found->passive                         ? LOSI_SOCKTYPE_LSTN :
	                                                  LOSI_SOCKTYPE_STRM));
	found->next = found->next->ai_next;
	return hasnext(found);
}

LOSIDRV_API losi_ErrorCode losiN_getcanonical(losi_NetDriver *drv,
                                              const char *name,
                                              char *buf, size_t sz)
{
	int err;
	struct addrinfo hints, *results;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	err = getaddrinfo(name, NULL, &hints, &results);
	if (!err) {
		const char *name = results->ai_canonname;
		if (name) {
			if (strlen(name) < sz) {
				strcpy(buf, name);
				err = 0;
			}
			else err = LOSI_ERRTOOMUCH;
		}
		else err = LOSI_ERRNOTFOUND;
		freeaddrinfo(results);
	}
	else err = getaddrerr(err);
	return err;
}

LOSIDRV_API losi_ErrorCode losiN_getaddrnames(losi_NetDriver *drv,
                                              losi_Address *addr,
                                              losi_AddressNameFlag flags,
                                              char *nodebuf, size_t nodesz,
                                              char *servbuf, size_t servsz)
{
	int err = getnameinfo((struct sockaddr*)addr, addrsz(addr),
	                      nodebuf, nodesz,
	                      servbuf, servsz,
	                      flags);
	if (!err) return LOSI_ERRNONE;
	switch (err) {
		case EAI_AGAIN:
		case EAI_NONAME: return LOSI_ERRNOTFOUND;
		case EAI_OVERFLOW: return LOSI_ERRTOOMUCH;
		case EAI_MEMORY: return LOSI_ERRNOMEMORY;
		case EAI_FAIL:
		case EAI_SYSTEM: return LOSI_ERRSYSTEMFAIL;
		case EAI_BADFLAGS:
		case EAI_FAMILY: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

