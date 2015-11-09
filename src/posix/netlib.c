#include "netlib.h"


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


#include <errno.h>
#include <signal.h> /* sigpipe handling */

LOSKIDRV_API int loski_opennetwork(loski_NetDriver *state)
{
	/* instals a handler to ignore sigpipe or it will crash us */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return errno;
	return 0;
}

LOSKIDRV_API int loski_closenetwork(loski_NetDriver *state)
{
	if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) return errno;
	return 0;
}


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


#include <string.h>

/* TODO: ASSERT(LOSKI_ADDRSIZE_IPV4 == sizeof(in_addr_t)) */

#define MAXLITERAL	16

LOSKIDRV_API int loskiN_initaddr(loski_NetDriver *state,
                                 loski_Address *address,
                                 loski_AddressType type)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (type != LOSKI_ADDRTYPE_IPV4) return 0;
	memset(address, 0, sizeof(loski_Address));
	addr->sin_family = AF_INET;
	return 1;
}

LOSKIDRV_API void loskiN_setaddrport(loski_NetDriver *state,
                                     loski_Address *address,
                                     loski_AddressPort port)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	addr->sin_port = htons(port);
}

LOSKIDRV_API void loskiN_setaddrbytes(loski_NetDriver *state,
                                      loski_Address *address,
                                      const char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	memcpy(&(addr->sin_addr.s_addr), data, LOSKI_ADDRSIZE_IPV4);
}

LOSKIDRV_API int loskiN_setaddrliteral(loski_NetDriver *state,
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

LOSKIDRV_API loski_AddressType loskiN_getaddrtype(loski_NetDriver *state,
                                                  loski_Address *address)
{
	return LOSKI_ADDRTYPE_IPV4;
}

LOSKIDRV_API loski_AddressPort loskiN_getaddrport(loski_NetDriver *state,
                                                  loski_Address *address)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return ntohs(addr->sin_port);
}

LOSKIDRV_API const char *loskiN_getaddrbytes(loski_NetDriver *state,
                                             loski_Address *address,
                                             size_t *sz)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (sz) *sz = LOSKI_ADDRSIZE_IPV4;
	return (const char *)(&(addr->sin_addr.s_addr));
}

LOSKIDRV_API const char *loskiN_getaddrliteral(loski_NetDriver *state,
                                               loski_Address *address,
                                               char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return inet_ntoa(addr->sin_addr);
}


/*****************************************************************************
 * Sockets *******************************************************************
 *****************************************************************************/


#include <unistd.h>
#include <netinet/tcp.h> /* TCP options (nagle algorithm disable) */
#include <fcntl.h> /* fnctnl function and associated constants */

LOSKIDRV_API int loskiN_initsock(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_getsockid(loski_NetDriver *state, loski_Socket *sock)
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

LOSKIDRV_API int loskiN_setsockopt(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_getsockopt(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_bindsock(loski_NetDriver *state,
                                 loski_Socket *sock,
                                 const loski_Address *address)
{
	if (bind(*sock, address, sizeof(loski_Address)) == 0) return 0; 
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

LOSKIDRV_API int loskiN_getsockaddr(loski_NetDriver *state,
                                    loski_Socket *sock,
                                    loski_Address *address,
                                    int peer)
{
	int err;
	socklen_t len = sizeof(loski_Address);
	if (peer) err = getpeername(*sock, address, &len);
	else      err = getsockname(*sock, address, &len);
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

LOSKIDRV_API int loskiN_connectsock(loski_NetDriver *state,
                                    loski_Socket *sock,
                                    const loski_Address *address)
{
	if (connect(*sock, address, sizeof(loski_Address)) == 0) return 0;
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

LOSKIDRV_API int loskiN_sendtosock(loski_NetDriver *state,
                                   loski_Socket *sock,
                                   const char *data,
                                   size_t size,
                                   size_t *bytes,
                                   const loski_Address *address)
{
	ssize_t err;
	if (address) err = sendto(*sock, data, size, 0,address,sizeof(loski_Address));
	else         err = send(*sock, data, size, 0);
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

LOSKIDRV_API int loskiN_recvfromsock(loski_NetDriver *state,
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
		err = recvfrom(*sock, buffer, size, flags, address, &len);
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

LOSKIDRV_API int loskiN_shutdownsock(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_acceptsock(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_listensock(loski_NetDriver *state,
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

LOSKIDRV_API int loskiN_closesock(loski_NetDriver *state,
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

#define LOSKI_EAFNOSUPPORT (-1)

LOSKIDRV_API void loskiN_freeaddrfound(loski_NetDriver *state,
                                       loski_AddressFound *found)
{
	if (found->results) {
		freeaddrinfo(found->results);
		found->results = NULL;
		found->next = NULL;
	}
}

static int hasnext(loski_AddressFound *found)
{
	while (found->next) {
		switch (found->next->ai_socktype) {
			case SOCK_DGRAM: found->nexttype = LOSKI_DGRMSOCKET; return 1;
			case SOCK_STREAM: found->nexttype = LOSKI_CONNSOCKET; return 1;
		}
		found->next = found->next->ai_next;
	}
	loskiN_freeaddrfound(NULL, found);
	return 0;
}

LOSKIDRV_API int loskiN_resolveaddr(loski_NetDriver *state,
                                    loski_AddressFound *found,
                                    loski_AddressFindFlag flags,
                                    const char *nodename,
                                    const char *servname)
{
	int err;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
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

LOSKIDRV_API int loskiN_getaddrfound(loski_NetDriver *state,
                                     loski_AddressFound *found,
                                     loski_Address *address,
                                     loski_SocketType *type)
{
	memcpy(address, found->next->ai_addr, found->next->ai_addrlen);
	*type = found->nexttype;
	found->next = found->next->ai_next;
	return hasnext(found);
}

