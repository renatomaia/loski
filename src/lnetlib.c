#include "lnetlib.h"
#include "loskiaux.h"

#include <string.h>
#include <ctype.h>


#ifdef LOSKI_DISABLE_NETSTATE
#define NETUPV	0
#define tonet(L)	NULL
#else
#define NETUPV	1
#define tonet(L)	((loski_NetState *)lua_touserdata(L, lua_upvalueindex(NETUPV)))
#endif


/*****************************************************************************
 * Addresses *****************************************************************
 *****************************************************************************/


static const char *str2port (const char *s, loski_AddressPort *pn) {
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

static loski_AddressPort int2port (lua_State *L, lua_Integer n, int idx){
	luaL_argcheck(L, 0 <= n && n <= LOSKI_ADDRMAXPORT, idx, "invalid port");
	return (loski_AddressPort)n;
}

static loski_Address *newaddr (lua_State *L) {
	loski_Address *na = (loski_Address *)lua_newuserdata(L,sizeof(loski_Address));
	luaL_setmetatable(L, LOSKI_NETADDRCLS);
	return na;
}

/* address = network.address([data [, port [, format]]]) */
static int net_address (lua_State *L) {
	loski_NetState *net = tonet(L);
	loski_Address *na;
	loski_AddressType type = LOSKI_ADDRTYPE_IPV4;
	loski_AddressPort port = 0;
	size_t sz = 0;
	const char *data = NULL;
	int binary = 0;
	int n = lua_gettop(L);
	if (n > 0) {
		data = luaL_checklstring(L, 1, &sz);
		if (n == 1) {  /* URI format */
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
			luaL_argcheck(L, str2port(c+1, &port) == e, 1, "invalid port");
		} else {
			const char *mode = luaL_optstring(L, 3, "t");
			port = int2port(L, luaL_checkinteger(L, 2), 2);
			if (mode[0] == 'b' && mode[1] == '\0') {  /* binary format */
				switch (sz) {
					case LOSKI_ADDRSIZE_IPV4: type = LOSKI_ADDRTYPE_IPV4; break;
					case LOSKI_ADDRSIZE_IPV6: type = LOSKI_ADDRTYPE_IPV6; break;
					default: luaL_argerror(L, 1, "invalid binary address");
				}
				binary = 1;
			} else if (mode[0] == 't' && mode[1] == '\0') {  /* literal format */
				type = strchr(data, ':') ? LOSKI_ADDRTYPE_IPV6 : LOSKI_ADDRTYPE_IPV4;
			} else {
				luaL_argerror(L, 3, "invalid mode");
			}
		}
	}
	na = newaddr(L);
	luaL_argcheck(L, loski_initaddress(net, na, type), 1, "unsupported address");
	if (data) {
		if (binary) loski_setaddrbytes(net, na, data);
		else luaL_argcheck(L, loski_setaddrliteral(net, na, data, sz), 1,
		                      "invalid literal address");
		if (port) loski_setaddrport(net, na, port);
	}
	return 1;
}


#define toaddr(L,i)	((loski_Address *)luaL_checkudata(L, i, LOSKI_NETADDRCLS))
#define optaddr(L,i)	((loski_Address *)luaL_testudata(L, i, LOSKI_NETADDRCLS))

/* uri = tostring(address) */
static int addr_tostring (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Address *na = toaddr(L, 1);
	loski_AddressType type = loski_getaddrtype(net, na);
	loski_AddressPort port = loski_getaddrport(net, na);
	char b[LOSKI_ADDRMAXLITERAL];
	const char *s = loski_getaddrliteral(net, na, b);
	if (s) lua_pushfstring(L, type==LOSKI_ADDRTYPE_IPV6?"[%s]:%d":"%s:%d",s,port);
	else lua_pushliteral(L, "<corrupted data>");
	return 1;
}


/* addr1 == addr2 */
static int addr_eq (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Address *a1 = optaddr(L, 1);
	loski_Address *a2 = optaddr(L, 2);
	if (a1 && a2 && (loski_getaddrtype(net, a1) == loski_getaddrtype(net, a2))
		           && (loski_getaddrport(net, a1) == loski_getaddrport(net, a2))) {
		size_t sz;
		const char *b = loski_getaddrbytes(net, a1, &sz);
		lua_pushboolean(L, memcmp(b, loski_getaddrbytes(net, a2, NULL), sz) == 0);
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
static int addr_index (lua_State *L) {
	static const char *const fields[] = {"port","binary","literal","type",NULL};
	loski_NetState *net = tonet(L);
	loski_Address *na = toaddr(L, 1);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			lua_pushinteger(L, loski_getaddrport(net, na));
		} break;
		case 1: {  /* binary */
			size_t sz;
			const char *s = loski_getaddrbytes(net, na, &sz);
			if (s) lua_pushlstring(L, s, sz);
			else lua_pushnil(L);
		} break;
		case 2: {  /* literal */
			char b[LOSKI_ADDRMAXLITERAL];
			const char *s = loski_getaddrliteral(net, na, b);
			if (s) lua_pushstring(L, s);
			else lua_pushnil(L);
		} break;
		case 3: {  /* type */
			loski_AddressType type = loski_getaddrtype(net, na);
			switch (type) {
				case LOSKI_ADDRTYPE_IPV4: lua_pushliteral(L, "ipv4"); break;
				case LOSKI_ADDRTYPE_IPV6: lua_pushliteral(L, "ipv6"); break;
				default: luaL_argerror(L, 1, "corrupted data");
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
static int addr_newindex (lua_State *L) {
	static const char *const fields[] = {"port","binary","literal","type",NULL};
	loski_NetState *net = tonet(L);
	loski_Address *na = toaddr(L, 1);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			loski_setaddrport(net, na, int2port(L, luaL_checkinteger(L, 3), 3));
		} break;
		case 1: {  /* binary */
			size_t sz, esz = 0;
			const char *data = luaL_checklstring(L, 3, &sz);
			loski_AddressType type = loski_getaddrtype(net, na);
			switch (type) {
				case LOSKI_ADDRTYPE_IPV4: esz = LOSKI_ADDRSIZE_IPV4; break;
				case LOSKI_ADDRTYPE_IPV6: esz = LOSKI_ADDRSIZE_IPV6; break;
				default: luaL_argerror(L, 1, "corrupted data");
			}
			luaL_argcheck(L, sz == esz, 3, "wrong byte size");
			loski_setaddrbytes(net, na, data);
		} break;
		case 2: {  /* literal */
			size_t sz;
			const char *data = luaL_checklstring(L, 3, &sz);
			luaL_argcheck(L, loski_setaddrliteral(net, na, data, sz), 3,
			                                      "invalid value");
		} break;
		case 3: {  /* type */
			int i = luaL_checkoption(L, 3, NULL, AddrTypeName);
			loski_AddressType type = loski_getaddrtype(net, na);
			loski_AddressType new = AddrTypeId[i];
			if (new != type)
				luaL_argcheck(L, loski_initaddress(net, na, new), 3,
				              "unsupported address");
		} break;
	}
	return 0;
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
	LOSKI_LSTNSOCKET,
	LOSKI_CONNSOCKET,
	LOSKI_DGRMSOCKET
};

static LuaSocket *newsock (lua_State *L, int type)
{
	LuaSocket *ls = (LuaSocket *)lua_newuserdata(L, sizeof(LuaSocket));
	ls->closed = 1;
	luaL_setmetatable(L, loski_SocketClasses[type]);
	return ls;
}

/* socket = network.socket(type [, domain]) */
static int net_socket (lua_State *L)
{
	loski_NetState *net = tonet(L);
	int optsck = luaL_checkoption(L, 1, NULL, SockTypeName);
	int optadr = luaL_checkoption(L, 2, "ipv4", AddrTypeName);
	loski_SocketType type = SockTypeId[optsck];
	loski_AddressType domain = AddrTypeId[optadr];
	LuaSocket *ls = newsock(L, type);
	int err = loski_createsocket(net, &ls->socket, type, domain);
	if (err == 0) ls->closed = 0;
	return luaL_doresults(L, 1, err);
}


#define tolsock(L,c)	((LuaSocket *)luaL_checkinstance(L, 1, \
                                                       loski_SocketClasses[c]))

/* string = tostring(socket) */
static int sck_tostring (lua_State *L)
{
	loski_NetState *net = tonet(L);
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	if (ls->closed)
		lua_pushliteral(L, "socket (closed)");
	else
		lua_pushfstring(L, "socket (%d)", loski_getsocketid(net, &ls->socket));
	return 1;
}


static int endsock (loski_NetState *net, lua_State *L, LuaSocket *ls)
{
	int err = loski_closesocket(net, &ls->socket);
	if (err == 0) ls->closed = 1;  /* mark socket as closed */
	return luaL_doresults(L, 0, err);
}

/* getmetatable(socket).__gc */
static int sck_gc (lua_State *L)
{
	loski_NetState *net = tonet(L);
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	if (!ls->closed) endsock(net, L, ls);
	return 0;
}


static loski_Socket *tosock (lua_State *L, int cls)
{
	LuaSocket *ls = tolsock(L, cls);
	if (ls->closed)
		luaL_error(L, "attempt to use a closed socket");
	return &ls->socket;
}

/* succ [, errmsg] = socket:close() */
static int sck_close (lua_State *L)
{
	loski_NetState *net = tonet(L);
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	tosock(L, LOSKI_BASESOCKET);  /* make sure argument is an open socket */
	return endsock(net, L, ls);
}


static loski_Address *checkaddress (lua_State *L, int idx)
{
	loski_Address *na = toaddr(L, idx);
	luaL_argcheck(L, na, idx, "address expected");
	return na;
}

/* succ [, errmsg] = socket:bind(address) */
static int sck_bind (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	const loski_Address *addr = checkaddress(L, 2);
	int err = loski_bindsocket(net, socket, addr);
	return luaL_doresults(L, 0, err);
}


/* address [, errmsg] = socket:getaddress(address [, site]) */
static int sck_getaddress (lua_State *L)
{
	static const char *const sites[] = {"local", "peer", NULL};
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address *addr = checkaddress(L, 2);
	int peer = luaL_checkoption(L, 3, "local", sites);
	int err = loski_socketaddress(net, socket, addr, peer);
	if (err == 0) lua_pushvalue(L, 2);
	return luaL_doresults(L, 1, err);
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
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val;
	int err = loski_getsocketoption(net, socket, opt, &val);
	if (err == 0) lua_pushboolean(L, val);
	return luaL_doresults(L, 1, err);
}

static int cnt_getoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val;
	int err = loski_getsocketoption(net, socket, opt, &val);
	if (err == 0) {
		if (opt == LOSKI_SOCKOPT_LINGER) lua_pushinteger(L, val);
		else                             lua_pushboolean(L, val);
	}
	return luaL_doresults(L, 1, err);
}

static int dgm_getoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val;
	int err = loski_getsocketoption(net, socket, opt, &val);
	if (err == 0) lua_pushboolean(L, val);
	return luaL_doresults(L, 1, err);
}


/* succ [, errmsg] = socket:setoption(name, value) */
static int lst_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val = lua_toboolean(L, 3);
	int err = loski_setsocketoption(net, socket, opt, val);
	return luaL_doresults(L, 0, err);
}

static int cnt_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val = (opt == LOSKI_SOCKOPT_LINGER) ? luaL_checkint(L, 3)
	                                        : lua_toboolean(L, 3);
	int err = loski_setsocketoption(net, socket, opt, val);
	return luaL_doresults(L, 0, err);
}

static int dgm_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val = lua_toboolean(L, 3);
	int err = loski_setsocketoption(net, socket, opt, val);
	return luaL_doresults(L, 0, err);
}


/* succ [, errmsg] = socket:connect(address) */
static int str_connect (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	const loski_Address *addr = checkaddress(L, 2);
	int err = loski_connectsocket(net, socket, addr);
	return luaL_doresults(L, 0, err);
}


static size_t posrelat (ptrdiff_t pos, size_t len)
{
	if (pos >= 0) return (size_t)pos;
	else if (0u - (size_t)pos > len) return 0;
	else return len - ((size_t)-pos) + 1;
}

static int senddata(lua_State *L, int cls, const loski_Address *addr)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, cls);
	size_t sz, sent;
	const char *data = luaL_checklstring(L, 2, &sz);
	size_t start = posrelat(luaL_optinteger(L, 3, 1), sz);
	size_t end = posrelat(luaL_optinteger(L, 4, -1), sz);
	int err;
	if (start < 1) start = 1;
	if (end > sz) end = sz;
	sz = end - start + 1;
	data += start - 1;
	err = loski_sendtosocket(net, socket, data, sz, &sent, addr);
	lua_pushinteger(L, sent);
	return luaL_doresults(L, 1, err);
}

/* sent [, errmsg] = socket:send(data [, i [, j]]) */
static int cnt_send (lua_State *L)
{
	return senddata(L, LOSKI_CONNSOCKET, NULL);
}

/* sent [, errmsg] = socket:send(data [, i [, j [, address]]]) */
static int dgm_send (lua_State *L)
{
	return senddata(L, LOSKI_DGRMSOCKET, optaddr(L, 5));
}


static int recvdata(lua_State *L, int cls, loski_Address *addr)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, cls);
	size_t len, sz = (size_t)luaL_checkinteger(L, 2);
	const char *mode = luaL_optstring(L, 3, "");
	loski_SocketRecvFlag flags = 0;
	luaL_Buffer lbuf;
	char *buf;
	int err;
	while (*mode) switch (*(mode++)) {
		case 'p': flags |= LOSKI_SOCKRCV_PEEKONLY; break;
		case 'a': flags |= LOSKI_SOCKRCV_WAITALL; break;
		default: luaL_error(L, "unknown mode char (got '%c')", *mode);
	}
	luaL_buffinit(L, &lbuf);
	buf = luaL_prepbuffsize(&lbuf, sz);  /* prepare buffer to read whole block */
	err = loski_recvfromsocket(net, socket, flags, buf, sz, &len, addr);
	if (err == 0) {
		luaL_addsize(&lbuf, len);
		luaL_pushresult(&lbuf);  /* close buffer */
	}
	return luaL_doresults(L, 1, err);
}

/* data [, errmsg] = socket:receive(size [, mode]) */
static int cnt_receive (lua_State *L)
{
	return recvdata(L, LOSKI_CONNSOCKET, NULL);
}

/* data [, errmsg] = socket:receive(size [, mode [, address]]) */
static int dgm_receive (lua_State *L)
{
	return recvdata(L, LOSKI_DGRMSOCKET, optaddr(L, 4));
}


/* succ [, errmsg] = socket:shutdown([mode]) */
static int cnt_shutdown (lua_State *L)
{
	static const char *const waynames[] = {"send", "receive", "both", NULL};
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	int way = luaL_checkoption(L, 2, "both", waynames);
	int err = loski_shutdownsocket(net, socket, way);
	return luaL_doresults(L, 0, err);
}


/* socket [, errmsg] = socket:accept([address]) */
static int lst_accept (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_Address *addr = optaddr(L, 2);
	LuaSocket *ls = newsock(L, LOSKI_CONNSOCKET);
	int err = loski_acceptsocket(net, socket, &ls->socket, addr);
	if (err == 0) ls->closed = 0;
	return luaL_doresults(L, 1, err);
}


/* succ [, errmsg] = socket:listen([backlog]) */
static int lst_listen (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	int backlog = luaL_optint(L, 2, 32);
	int err = loski_listensocket(net, socket, backlog);
	return luaL_doresults(L, 0, err);
}


/*****************************************************************************
 * Names *********************************************************************
 *****************************************************************************/

#define FOUNDUPV	NETUPV+1
#define tofound(L) \
	((loski_AddressFound *)lua_touserdata(L, lua_upvalueindex(FOUNDUPV)))

/* [address, socktype, more =] next([address]) */
static int net_nextfound (lua_State *L) {
	loski_NetState *net = tonet(L);
	loski_AddressFound *found = tofound(L);
	if (found) {
		int more, i;
		loski_SocketType type;
		loski_Address *addr = optaddr(L, 1);
		if (!addr) {
			lua_settop(L, 0);
			addr = newaddr(L);
		} else {
			lua_settop(L, 1);
		}
		more = loski_netgetaddrfound(net, found, addr, &type);
		for (i = 0; SockTypeName[i] && SockTypeId[i] != type; ++i);
		lua_pushstring(L, SockTypeName[i]);
		lua_pushboolean(L, more);
		if (!more) {  /* avoid further attempts to get more results */
			lua_pushnil(L);
			lua_replace(L, FOUNDUPV);
		}
		return 3;
	}
	return 0;
}

static int net_freefound (lua_State *L) {
	loski_NetState *net = tonet(L);
	loski_AddressFound *found = tofound(L);
	loski_netfreeaddrfound(net, found);
	return 0;
}

/* next [, errmsg] = dns.resolve (host [, service [, mode]]) */
static int net_resolve (lua_State *L) {
	loski_NetState *net = tonet(L);
	const char *nodename = luaL_checkstring(L, 1);
	const char *servname = luaL_optstring(L, 2, NULL);
	const char *mode = luaL_optstring(L, 2, "");
	loski_AddressFindFlag flags = 0;
	loski_AddressFound *found;
	int err;
	while (*mode) switch (*(mode++)) {
		case 'l': flags |= LOSKI_ADDRFIND_LOCAL; break;
		case '4': flags |= LOSKI_ADDRFIND_IPV4; break;
		case '6': flags |= LOSKI_ADDRFIND_IPV6; break;
		case 'm': flags |= LOSKI_ADDRFIND_MAPPED; break;
		case 'd': flags |= LOSKI_ADDRFIND_DGRM; break;
		case 's': flags |= LOSKI_ADDRFIND_STRM; break;
		default: luaL_error(L, "unknown mode char (got '%c')", *mode);
	}
#ifndef LOSKI_DISABLE_NETSTATE
	lua_pushvalue(L, lua_upvalueindex(NETUPV)); /* push library state */
#endif
	found = (loski_AddressFound *)luaL_newsentinel(L, sizeof(loski_AddressFound),
	                                                  net_freefound);
	err = loski_netresolveaddr(net, found, flags, nodename, servname);
	if (err) luaL_cancelsentinel(L);
	else lua_pushcclosure(L, net_nextfound, FOUNDUPV);
	return luaL_doresults(L, 1, err);
}


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


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
	{"resolve", net_resolve},
	{"socket", net_socket},
	{NULL, NULL}
};

#ifndef LOSKI_DISABLE_NETSTATE
static int net_sentinel (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_closenetwork(net);
	return 0;
}
#endif

LUAMOD_API int luaopen_network (lua_State *L)
{
#ifndef LOSKI_DISABLE_NETSTATE
	/* create sentinel */
	lua_settop(L, 0);  /* dicard any arguments */
	loski_NetState *net = (loski_NetState *)luaL_newsentinel(L,
	                                        sizeof(loski_NetState),
	                                        net_sentinel);
	/* initialize library */
	if (loski_opennetwork(net) != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create address class */
	pushsentinel(L);
	luaL_newclass(L, LOSKI_NETADDRCLS, NULL, addr, NETUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_BASESOCKET], NULL, sck, NETUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP listening socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_LSTNSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], lst, NETUPV);
	lua_pushliteral(L, "listen");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create streaming socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_STRMSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], str, NETUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP connection socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_CONNSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], cnt, NETUPV);
	lua_pushliteral(L, "connection");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create UDP socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_DGRMSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], dgm, NETUPV);
	lua_pushliteral(L, "datagram");
	lua_setfield(L, -2, "type");
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, NETUPV);
	return 1;
}
