#include "luaosi/lnetlib.h"


#include "luaosi/lauxlib.h"
#include "luaosi/levtlib.h"

#include <string.h>
#include <ctype.h>


#ifdef LOSKI_DISABLE_NETDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)	((loski_NetDriver *)lua_touserdata(L, \
                                      lua_upvalueindex(DRVUPV)))
#endif


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


static const char *str2port (const char *s, loski_AddressPort *pn)
{
	lua_Unsigned n = 0;
	if (!isdigit((unsigned char)*s)) return NULL;  /* no digit? */
	do {
		n = n * 10 + (*s - '0');
		if (n > LOSKI_ADDRMAXPORT) return NULL;  /* value too large */
		++s;
	} while (isdigit((unsigned char)*s));
	*pn = (loski_AddressPort)n;
	return s;
}

static loski_AddressPort int2port (lua_State *L, lua_Integer n, int idx)
{
	luaL_argcheck(L, 0 <= n && n <= LOSKI_ADDRMAXPORT, idx, "invalid port");
	return (loski_AddressPort)n;
}

static int pushnewaddr (lua_State *L,
                        loski_AddressType type,
                        loski_AddressPort port,
                        const char *data, int bin)
{
	loski_NetDriver *drv = todrv(L);
	loski_Address *na = loski_newaddress(L);
	loski_ErrorCode err = loskiN_initaddr(drv, na, type);
	if (!err && data) {
		if (bin) err = loskiN_setaddrbytes(drv, na, data);
		else err = loskiN_setaddrliteral(drv, na, data);
		if (!err && port) err = loskiN_setaddrport(drv, na, port);
	}
	return loskiL_doresults(L, 1, err);
}

/* address = network.address([data [, port [, format]]]) */
static int net_address (lua_State *L)
{
	loski_AddressType type = LOSKI_ADDRTYPE_IPV4;
	loski_AddressPort port = 0;
	const char *data = NULL;
	int bin = 0, n = lua_gettop(L);
	if (n > 0) {
		size_t sz;
		data = luaL_checklstring(L, 1, &sz);
		if (n == 1) {  /* URI format */
			char literal[LOSKI_ADDRMAXLITERAL];
			const char *c, *e = data + sz;
			if (data[0] == '[') {
				c = strchr(++data, ']');
				luaL_argcheck(L, c && c[1] == ':', 1, "invalid URI format");
				sz = c - data;
				type = LOSKI_ADDRTYPE_IPV6;
				++c;
			} else {
				c = strchr(data, ':');
				luaL_argcheck(L, c, 1, "invalid URI format");
				sz = c - data;
				type = LOSKI_ADDRTYPE_IPV4;
			}
			luaL_argcheck(L, sz < LOSKI_ADDRMAXLITERAL, 1, "invalid URI format");
			luaL_argcheck(L, str2port(c+1, &port) == e, 1, "invalid port");
			memcpy(literal, data, sz);
			literal[sz] = '\0';
			return pushnewaddr(L, type, port, literal, 0);
		} else {
			const char *mode = luaL_optstring(L, 3, "t");
			port = int2port(L, luaL_checkinteger(L, 2), 2);
			if (mode[0] == 'b' && mode[1] == '\0') {  /* binary format */
				switch (sz) {
					case LOSKI_ADDRSIZE_IPV4: type = LOSKI_ADDRTYPE_IPV4; break;
					case LOSKI_ADDRSIZE_IPV6: type = LOSKI_ADDRTYPE_IPV6; break;
					default: return luaL_argerror(L, 1, "invalid binary address");
				}
				bin = 1;
			} else if (mode[0] == 't' && mode[1] == '\0') {  /* literal format */
				type = strchr(data, ':') ? LOSKI_ADDRTYPE_IPV6 : LOSKI_ADDRTYPE_IPV4;
			} else {
				return luaL_argerror(L, 3, "invalid mode");
			}
		}
	}
	return pushnewaddr(L, type, port, data, bin);
}


#define toaddr(L,i)	((loski_Address *)luaL_checkudata(L, i, LOSKI_NETADDRCLS))

/* uri = tostring(address) */
static int addr_tostring (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Address *na = toaddr(L, 1);
	loski_AddressType type = loskiN_getaddrtype(drv, na);
	loski_AddressPort port = loskiN_getaddrport(drv, na);
	char b[LOSKI_ADDRMAXLITERAL];
	const char *s = loskiN_getaddrliteral(drv, na, b);
	if (s) lua_pushfstring(L, type==LOSKI_ADDRTYPE_IPV6?"[%s]:%d":"%s:%d",s,port);
	else lua_pushfstring(L, "invalid address type (%d)", type);
	return 1;
}


/* addr1 == addr2 */
static int addr_eq (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Address *a1 = loski_toaddress(L, 1);
	loski_Address *a2 = loski_toaddress(L, 2);
	if ( a1 && a2 &&
	     (loskiN_getaddrtype(drv, a1) == loskiN_getaddrtype(drv, a2)) &&
	     (loskiN_getaddrport(drv, a1) == loskiN_getaddrport(drv, a2)) ) {
		size_t sz;
		const char *b = loskiN_getaddrbytes(drv, a1, &sz);
		lua_pushboolean(L, memcmp(b, loskiN_getaddrbytes(drv, a2, NULL), sz)==0);
	}
	else lua_pushboolean(L, 0);
	return 1;
}


/*
 * type = address.type
 * literal = address.literal
 * binary = address.binary
 * port = address.port
 */
static int addr_index (lua_State *L)
{
	static const char *const fields[] = {"port","binary","literal","type",NULL};
	loski_NetDriver *drv = todrv(L);
	loski_Address *na = toaddr(L, 1);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			lua_pushinteger(L, loskiN_getaddrport(drv, na));
		} break;
		case 1: {  /* binary */
			size_t sz;
			const char *s = loskiN_getaddrbytes(drv, na, &sz);
			if (s) lua_pushlstring(L, s, sz);
			else lua_pushnil(L);
		} break;
		case 2: {  /* literal */
			char b[LOSKI_ADDRMAXLITERAL];
			const char *s = loskiN_getaddrliteral(drv, na, b);
			lua_pushstring(L, s);
		} break;
		case 3: {  /* type */
			loski_AddressType type = loskiN_getaddrtype(drv, na);
			switch (type) {
				case LOSKI_ADDRTYPE_IPV4: lua_pushliteral(L, "ipv4"); break;
				case LOSKI_ADDRTYPE_IPV6: lua_pushliteral(L, "ipv6"); break;
				default: return luaL_argerror(L, 1, "corrupted data");
			}
		} break;
	}
	return 1;
}


static const char *const AddrTypeName[] = {
	"ipv4",
	"ipv6",
	NULL
};
static const loski_AddressType AddrTypeId[] = {
	LOSKI_ADDRTYPE_IPV4,
	LOSKI_ADDRTYPE_IPV6
};

/*
 * address.type = type
 * address.literal = literal
 * address.binary = binary
 * address.port = port
 */
static int addr_newindex (lua_State *L)
{
	static const char *const fields[] = {"port","binary","literal","type",NULL};
	loski_NetDriver *drv = todrv(L);
	loski_Address *na = toaddr(L, 1);
	int field = luaL_checkoption(L, 2, NULL, fields);
	loski_ErrorCode err = 0;
	switch (field) {
		case 0: {  /* port */
			err = loskiN_setaddrport(drv, na, int2port(L, luaL_checkinteger(L,3), 3));
		} break;
		case 1: {  /* binary */
			size_t sz, esz = 0;
			const char *data = luaL_checklstring(L, 3, &sz);
			loski_AddressType type = loskiN_getaddrtype(drv, na);
			switch (type) {
				case LOSKI_ADDRTYPE_IPV4: esz = LOSKI_ADDRSIZE_IPV4; break;
				case LOSKI_ADDRTYPE_IPV6: esz = LOSKI_ADDRSIZE_IPV6; break;
				default: return luaL_argerror(L, 1, "corrupted data");
			}
			luaL_argcheck(L, sz == esz, 3, "wrong byte size");
			err = loskiN_setaddrbytes(drv, na, data);
		} break;
		case 2: {  /* literal */
			size_t sz;
			const char *data = luaL_checklstring(L, 3, &sz);
			err = loskiN_setaddrliteral(drv, na, data);
		} break;
		case 3: {  /* type */
			int i = luaL_checkoption(L, 3, NULL, AddrTypeName);
			loski_AddressType type = loskiN_getaddrtype(drv, na);
			loski_AddressType new = AddrTypeId[i];
			if (new != type) err = loskiN_initaddr(drv, na, new);
		} break;
	}
	if (!err) return 0;
	loskiL_pusherrmsg(L, err);
	return lua_error(L);
}


/*****************************************************************************
 * Sockets *******************************************************************
 *****************************************************************************/


static const char *const SockTypeName[] = {
	"listen",
	"connection",
	"datagram",
	NULL
};
static const loski_SocketType SockTypeId[] = {
	LOSKI_SOCKTYPE_LSTN,
	LOSKI_SOCKTYPE_CONN,
	LOSKI_SOCKTYPE_DGRM
};

/* socket = network.socket(type [, domain]) */
static int net_socket (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	int optsck = luaL_checkoption(L, 1, NULL, SockTypeName);
	int optadr = luaL_checkoption(L, 2, "ipv4", AddrTypeName);
	loski_SocketType type = SockTypeId[optsck];
	loski_AddressType domain = AddrTypeId[optadr];
	loski_Socket *socket = loski_newsocket(L, type);
	loski_ErrorCode err = loskiN_initsock(drv, socket, type, domain);
	if (!err) loski_enablesocket(L, -1);
	return loskiL_doresults(L, 1, err);
}


#define tolsock(L,c)	((LuaSocket *)luaL_checkinstance(L, 1, \
                                                       loski_SocketClasses[c]))

/* string = tostring(socket) */
static int sck_tostring (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	LuaSocket *ls = tolsock(L, LOSKI_SOCKTYPE_SOCK);
	if (ls->closed)
		lua_pushliteral(L, "socket (closed)");
	else
		lua_pushfstring(L, "socket (%d)", loskiN_getsockid(drv, &ls->socket));
	return 1;
}


static int endsock (loski_NetDriver *drv, lua_State *L, LuaSocket *ls)
{
	loski_ErrorCode err = loskiN_closesock(drv, &ls->socket);
	if (!err) ls->closed = 1;  /* mark socket as closed */
	return loskiL_doresults(L, 0, err);
}

/* getmetatable(socket).__gc(socket) */
static int sck_gc (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	LuaSocket *ls = tolsock(L, LOSKI_SOCKTYPE_SOCK);
	if (!ls->closed) endsock(drv, L, ls);
	return 0;
}


static loski_Socket *tosock (lua_State *L, int cls)
{
	LuaSocket *ls = tolsock(L, cls);
	if (ls->closed) luaL_error(L, "attempt to use a closed socket");
	return &ls->socket;
}

/* succ [, errmsg] = socket:close() */
static int sck_close (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	LuaSocket *ls = tolsock(L, LOSKI_SOCKTYPE_SOCK);
	tosock(L, LOSKI_SOCKTYPE_SOCK);  /* make sure argument is an open socket */
	return endsock(drv, L, ls);
}


/* succ [, errmsg] = socket:bind(address) */
static int sck_bind (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_SOCK);
	const loski_Address *addr = toaddr(L, 2);
	loski_ErrorCode err = loskiN_bindsock(drv, socket, addr);
	return loskiL_doresults(L, 0, err);
}


/* address [, errmsg] = socket:getaddress([site [, address]]) */
static int sck_getaddress (lua_State *L)
{
	static const char *const sites[] = {"this", "peer", NULL};
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_SOCK);
	int peer = luaL_checkoption(L, 2, "this", sites);
	loski_Address *addr = loski_toaddress(L, 3);
	loski_ErrorCode err;
	if (!addr) {
		lua_settop(L, 2);
		addr = loski_newaddress(L);
	}
	else lua_settop(L, 3);
	err = loskiN_getsockaddr(drv, socket, addr, peer);
	return loskiL_doresults(L, 1, err);
}


struct SocketOptionInfo {
	const char *name;
	loski_SocketOption option;
};

static struct SocketOptionInfo lst_opts[] = {
	{"blocking", LOSKI_SOCKOPT_BLOCKING},
	{"reuseaddr", LOSKI_SOCKOPT_REUSEADDR},
	{"dontroute", LOSKI_SOCKOPT_DONTROUTE},
	{NULL, 0}
};

static struct SocketOptionInfo cnt_opts[] = {
	{"blocking", LOSKI_SOCKOPT_BLOCKING},
	{"reuseaddr", LOSKI_SOCKOPT_REUSEADDR},
	{"dontroute", LOSKI_SOCKOPT_DONTROUTE},
	{"linger", LOSKI_SOCKOPT_LINGER},
	{"keepalive", LOSKI_SOCKOPT_KEEPALIVE},
	{"nodelay", LOSKI_SOCKOPT_TCPNDELAY},
	{NULL, 0}
};

static struct SocketOptionInfo dgm_opts[] = {
	{"blocking", LOSKI_SOCKOPT_BLOCKING},
	{"reuseaddr", LOSKI_SOCKOPT_REUSEADDR},
	{"dontroute", LOSKI_SOCKOPT_DONTROUTE},
	{"broadcast", LOSKI_SOCKOPT_BROADCAST},
	{NULL, 0}
};

static int checksockopt (lua_State *L,
                         int narg,
                         struct SocketOptionInfo opts[])
{
	const char *name = luaL_checkstring(L, narg);
	int i;
	for (i=0; opts[i].name; i++)
		if (strcmp(opts[i].name, name) == 0)
			return opts[i].option;
	return luaL_argerror(L, narg,
	                     lua_pushfstring(L, "invalid option " LUA_QS, name));
}


/* value [, errmsg] = socket:getoption(name) */
static int lst_getoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_LSTN);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val;
	loski_ErrorCode err = loskiN_getsockopt(drv, socket, opt, &val);
	if (err == 0) lua_pushboolean(L, val);
	return loskiL_doresults(L, 1, err);
}

static int cnt_getoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_CONN);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val;
	loski_ErrorCode err = loskiN_getsockopt(drv, socket, opt, &val);
	if (err == 0) {
		if (opt == LOSKI_SOCKOPT_LINGER) lua_pushinteger(L, val);
		else                             lua_pushboolean(L, val);
	}
	return loskiL_doresults(L, 1, err);
}

static int dgm_getoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_DGRM);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val;
	loski_ErrorCode err = loskiN_getsockopt(drv, socket, opt, &val);
	if (err == 0) lua_pushboolean(L, val);
	return loskiL_doresults(L, 1, err);
}


/* succ [, errmsg] = socket:setoption(name, value) */
static int lst_setoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_LSTN);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val = lua_toboolean(L, 3);
	loski_ErrorCode err = loskiN_setsockopt(drv, socket, opt, val);
	return loskiL_doresults(L, 0, err);
}

static int cnt_setoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_CONN);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val = (opt == LOSKI_SOCKOPT_LINGER) ? luaL_checkinteger(L, 3)
	                                        : lua_toboolean(L, 3);
	loski_ErrorCode err = loskiN_setsockopt(drv, socket, opt, val);
	return loskiL_doresults(L, 0, err);
}

static int dgm_setoption (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_DGRM);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val = lua_toboolean(L, 3);
	loski_ErrorCode err = loskiN_setsockopt(drv, socket, opt, val);
	return loskiL_doresults(L, 0, err);
}


/* succ [, errmsg] = socket:connect(address) */
static int str_connect (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_STRM);
	const loski_Address *addr = toaddr(L, 2);
	loski_ErrorCode err = loskiN_connectsock(drv, socket, addr);
	return loskiL_doresults(L, 0, err);
}


static size_t posrelat (ptrdiff_t pos, size_t len)
{
	if (pos >= 0) return (size_t)pos;
	else if (0u - (size_t)pos > len) return 0;
	else return len - ((size_t)-pos) + 1;
}

static int senddata(lua_State *L, int cls, const loski_Address *addr)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, cls);
	size_t sz, sent;
	const char *data = luaL_checklstring(L, 2, &sz);
	size_t start = posrelat(luaL_optinteger(L, 3, 1), sz);
	size_t end = posrelat(luaL_optinteger(L, 4, -1), sz);
	loski_ErrorCode err;
	if (start < 1) start = 1;
	if (end > sz) end = sz;
	sz = end - start + 1;
	data += start - 1;
	err = loskiN_sendtosock(drv, socket, data, sz, &sent, addr);
	lua_pushinteger(L, sent);
	return loskiL_doresults(L, 1, err);
}

/* sent [, errmsg] = socket:send(data [, i [, j]]) */
static int cnt_send (lua_State *L)
{
	return senddata(L, LOSKI_SOCKTYPE_CONN, NULL);
}

/* sent [, errmsg] = socket:send(data [, i [, j [, address]]]) */
static int dgm_send (lua_State *L)
{
	return senddata(L, LOSKI_SOCKTYPE_DGRM, loski_toaddress(L, 5));
}


static int recvdata(lua_State *L, int cls, loski_Address *addr)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, cls);
	size_t len, sz = (size_t)luaL_checkinteger(L, 2);
	const char *mode = luaL_optstring(L, 3, "");
	loski_SocketRecvFlag flags = 0;
	luaL_Buffer lbuf;
	char *buf;
	loski_ErrorCode err;
	for (; *mode; ++mode) switch (*mode) {
		case 'p': flags |= LOSKI_SOCKRCV_PEEKONLY; break;
		case 'a': flags |= LOSKI_SOCKRCV_WAITALL; break;
		default: return luaL_error(L, "unknown mode char (got '%c')", *mode);
	}
	luaL_buffinit(L, &lbuf);
	buf = luaL_prepbuffsize(&lbuf, sz);  /* prepare buffer to read whole block */
	err = loskiN_recvfromsock(drv, socket, flags, buf, sz, &len, addr);
	if (!err) {
		luaL_addsize(&lbuf, len);
		luaL_pushresult(&lbuf);  /* close buffer */
	}
	return loskiL_doresults(L, 1, err);
}

/* data [, errmsg] = socket:receive(size [, mode]) */
static int cnt_receive (lua_State *L)
{
	return recvdata(L, LOSKI_SOCKTYPE_CONN, NULL);
}

/* data [, errmsg] = socket:receive(size [, mode [, address]]) */
static int dgm_receive (lua_State *L)
{
	return recvdata(L, LOSKI_SOCKTYPE_DGRM, loski_toaddress(L, 4));
}


/* succ [, errmsg] = socket:shutdown([mode]) */
static int cnt_shutdown (lua_State *L)
{
	static const char *const waynames[] = {"send", "receive", "both", NULL};
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_CONN);
	int way = luaL_checkoption(L, 2, "both", waynames);
	loski_ErrorCode err = loskiN_shutdownsock(drv, socket, way);
	return loskiL_doresults(L, 0, err);
}


/* socket [, errmsg] = socket:accept([address]) */
static int lst_accept (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_LSTN);
	loski_Address *addr = loski_toaddress(L, 2);
	loski_Socket *conn = loski_newsocket(L, LOSKI_SOCKTYPE_CONN);
	loski_ErrorCode err = loskiN_acceptsock(drv, socket, conn, addr);
	if (!err) loski_enablesocket(L, -1);
	return loskiL_doresults(L, 1, err);
}


/* succ [, errmsg] = socket:listen([backlog]) */
static int lst_listen (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_Socket *socket = tosock(L, LOSKI_SOCKTYPE_LSTN);
	int backlog = luaL_optinteger(L, 2, 32);
	loski_ErrorCode err = loskiN_listensock(drv, socket, backlog);
	return loskiL_doresults(L, 0, err);
}


/*****************************************************************************
 * Names *********************************************************************
 *****************************************************************************/


#define FOUNDUPV	DRVUPV+1
#define tofound(L) \
	((loski_AddressFound *)lua_touserdata(L, lua_upvalueindex(FOUNDUPV)))

/* [address, socktype, more =] next([address]) */
static int net_nextfound (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_AddressFound *found = tofound(L);
	if (found) {
		int more, i;
		loski_SocketType type;
		loski_Address *addr = loski_toaddress(L, 1);
		if (!addr) {
			lua_settop(L, 0);
			addr = loski_newaddress(L);
		}
		else lua_settop(L, 1);
		more = loskiN_getaddrfound(drv, found, addr, &type);
		for (i = 0; SockTypeName[i] && SockTypeId[i] != type; ++i);
		lua_pushstring(L, SockTypeName[i]);
		lua_pushboolean(L, more);
		if (!more) {  /* avoid further attempts to get more results */
			lua_pushnil(L);
			lua_replace(L, lua_upvalueindex(FOUNDUPV));
		}
		return 3;
	}
	return 0;
}

static int net_freefound (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	loski_AddressFound *found = (loski_AddressFound *)lua_touserdata(L, 1);
	loskiN_freeaddrfound(drv, found);
	return 0;
}

/* next [, errmsg] = network.resolve (name [, service [, mode]]) */
static int net_resolve (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	const char *nodename = luaL_optstring(L, 1, NULL);
	const char *servname = luaL_optstring(L, 2, NULL);
	const char *mode = luaL_optstring(L, 3, "");
	loski_AddressFindFlag flags = 0;
	loski_AddressFound *found;
	loski_ErrorCode err;
	if (nodename) {
		if (nodename[0] == '*' && nodename[1] == '\0') {
			luaL_argcheck(L, servname, 2, "service must be provided for '*'");
			flags |= LOSKI_ADDRFIND_LSTN;
			nodename = NULL;
		}
	}
	else luaL_argcheck(L, servname, 1, "name or service must be provided");
	for (; *mode; ++mode) switch (*mode) {
		case '4': flags |= LOSKI_ADDRFIND_IPV4; break;
		case '6': flags |= LOSKI_ADDRFIND_IPV6; break;
		case 'm': flags |= LOSKI_ADDRFIND_MAP4; break;
		case 'd': flags |= LOSKI_ADDRFIND_DGRM; break;
		case 's': flags |= LOSKI_ADDRFIND_STRM; break;
		default: return luaL_error(L, "unknown mode char (got '%c')", *mode);
	}
#ifndef LOSKI_DISABLE_NETDRV
	lua_pushvalue(L, lua_upvalueindex(DRVUPV)); /* push library state */
#endif
	found = (loski_AddressFound *)luaL_newsentinel(L, sizeof(loski_AddressFound),
	                                                  net_freefound);
	err = loskiN_resolveaddr(drv, found, flags, nodename, servname);
	if (err) luaL_cancelsentinel(L);
	else lua_pushcclosure(L, net_nextfound, FOUNDUPV);
	return loskiL_doresults(L, 1, err);
}

static char *incbuf (lua_State *L, size_t *sz)
{
	lua_pop(L, 1);  /* remove old buffer */
	*sz += LOSKI_NETNAMEBUFSZ;
	return (char *)lua_newuserdata(L, *sz);
}

/* name [, service] = network.getname (data [, mode]) */
static int net_getname (lua_State *L)
{
	loski_NetDriver *drv = todrv(L);
	char buffer[LOSKI_NETNAMEBUFSZ];
	size_t sz = LOSKI_NETNAMEBUFSZ;
	char *buf = buffer;
	loski_ErrorCode err;
	int ltype = lua_type(L, 1);
	lua_settop(L, 2);  /* discard any extra parameters */
	lua_pushnil(L);  /* simulate the initial buffer on the stack */
	if (ltype == LUA_TSTRING) {
		do {
			err = loskiN_getcanonical(drv, lua_tostring(L, 1), buf, sz);
			if (!err) {
				lua_pushstring(L, buf);
				return 1;
			} else if ((err == LOSKI_ERRTOOMUCH) && (buf = incbuf(L, &sz))) {
				err = 0;
			}
		} while (!err);
	} else {
		loski_Address *na;
		loski_AddressNameFlag flags = 0;
		const char *mode = luaL_optstring(L, 2, "");
		for (; *mode; ++mode) switch (*mode) {
			case 'l': flags |= LOSKI_ADDRNAME_LOCAL; break;
			case 'd': flags |= LOSKI_ADDRNAME_DGRM; break;
			default: return luaL_error(L, "unknown mode char (got '%c')", *mode);
		}
		if (ltype == LUA_TNUMBER) {
			loski_AddressPort port = int2port(L, luaL_checkinteger(L, 1), 1);
			na = loski_newaddress(L);
			if ( !(err = loskiN_initaddr(drv, na, LOSKI_ADDRTYPE_IPV4)) &&
			     !(err = loskiN_setaddrport(drv, na, port)) ) {
				do {
					err = loskiN_getaddrnames(drv, na, flags, NULL, 0, buf, sz);
					if (!err) {
						lua_pushstring(L, buf);
						return 1;
					} else if ((err == LOSKI_ERRTOOMUCH) && (buf = incbuf(L, &sz))) {
						err = 0;
					}
				} while (!err);
			}
		} else {
			na = toaddr(L, 1);
			do {
				size_t ssz = sz/4;
				size_t nsz = sz-ssz;
				char *sbuf = buf+nsz;
				err = loskiN_getaddrnames(drv, na, flags, buf, nsz, sbuf, ssz);
				if (!err) {
					lua_pushstring(L, buf);
					lua_pushstring(L, sbuf);
					return 2;
				} else if ((err == LOSKI_ERRTOOMUCH) && (buf = incbuf(L, &sz))) {
					err = 0;
				}
			} while (!err);
		}
	}
	return loskiL_doresults(L, 0, err);
}


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


static int net_type (lua_State *L) {
	if (lua_isuserdata(L, 1) && lua_getmetatable(L, 1)) {
		luaL_getmetatable(L, LOSKI_NETADDRCLS);
		if (lua_rawequal(L, 2, 3)) {
			lua_pushliteral(L, "address");
			return 1;
		}
		lua_pop(L, 1);  /* remove address metatable */
		if (luaL_issubclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_SOCK])) {
			lua_pushliteral(L, "socket");
			return 1;
		}
	}
	return 0;
}


static const luaL_Reg addr[] = {
	{"__tostring", addr_tostring},
	{"__eq", addr_eq},
	{"__index", addr_index},
	{"__newindex", addr_newindex},
	{NULL, NULL}
};

static const luaL_Reg sck[] = {
	{"__tostring", sck_tostring},
	{"__gc", sck_gc},
	{"close", sck_close},
	{"bind", sck_bind},
	{"getaddress", sck_getaddress},
	{NULL, NULL}
};

static const luaL_Reg lst[] = {
	{"getoption", lst_getoption},
	{"setoption", lst_setoption},
	{"accept", lst_accept},
	{"listen", lst_listen},
	{NULL, NULL}
};

static const luaL_Reg str[] = {
	{"connect", str_connect},
	{NULL, NULL}
};

static const luaL_Reg cnt[] = {
	{"getoption", cnt_getoption},
	{"setoption", cnt_setoption},
	{"send", cnt_send},
	{"receive", cnt_receive},
	{"shutdown", cnt_shutdown},
	{NULL, NULL}
};

static const luaL_Reg dgm[] = {
	{"getoption", dgm_getoption},
	{"setoption", dgm_setoption},
	{"send", dgm_send},
	{"receive", dgm_receive},
	{NULL, NULL}
};

static const luaL_Reg lib[] = {
	{"address", net_address},
	{"getname", net_getname},
	{"resolve", net_resolve},
	{"socket", net_socket},
	{"type", net_type},
	{NULL, NULL}
};

#ifndef LOSKI_DISABLE_NETDRV
static int lfreedrv (lua_State *L)
{
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, 1);
	loskiN_freedrv(drv);
	return 0;
}
#endif

LUAMOD_API int luaopen_network (lua_State *L)
{
#ifndef LOSKI_DISABLE_NETDRV
	/* create sentinel */
	loski_NetDriver *drv;
	loski_ErrorCode err;
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (loski_NetDriver *)luaL_newsentinel(L, sizeof(loski_NetDriver),
	                                             lfreedrv);
	/* initialize library */
	err = loskiN_initdrv(drv);
	if (err) {
		luaL_cancelsentinel(L);
		loskiL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create address class */
	pushsentinel(L);
	luaL_newclass(L, LOSKI_NETADDRCLS, NULL, addr, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_SOCK], NULL, sck, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP listening socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_LSTN],
	                 loski_SocketClasses[LOSKI_SOCKTYPE_SOCK], lst, DRVUPV);
	lua_pushliteral(L, "listen");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create streaming socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_STRM],
	                 loski_SocketClasses[LOSKI_SOCKTYPE_SOCK], str, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP connection socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_CONN],
	                 loski_SocketClasses[LOSKI_SOCKTYPE_STRM], cnt, DRVUPV);
	lua_pushliteral(L, "connection");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create UDP socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_SOCKTYPE_DGRM],
	                 loski_SocketClasses[LOSKI_SOCKTYPE_STRM], dgm, DRVUPV);
	lua_pushliteral(L, "datagram");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);

#ifdef LOSKI_ENABLE_SOCKETEVENTS
	loski_seteventsourceconv(L, loski_SocketClasses[LOSKI_SOCKTYPE_LSTN],
	                            loskiN_socket2evtsrc);
	loski_seteventsourceconv(L, loski_SocketClasses[LOSKI_SOCKTYPE_CONN],
	                            loskiN_socket2evtsrc);
	loski_seteventsourceconv(L, loski_SocketClasses[LOSKI_SOCKTYPE_DGRM],
	                            loskiN_socket2evtsrc);
#endif
	return 1;
}
