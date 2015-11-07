#include "lnetlib.h"
#include "laddrlib.h"
#include "loskiaux.h"

#include <string.h>


#define pushsockres(L,n,e) luaL_pushresults(L,n,e,loski_socketerror)


#ifdef LOSKI_DISABLE_NETSTATE
#define tonet(L)	NULL
#else
#define tonet(L)	((loski_NetState *)lua_touserdata(L, lua_upvalueindex(1)))
#endif


typedef struct LuaSocket {
	loski_Socket socket;
	int closed;
} LuaSocket;

static LuaSocket *newsock (lua_State *L, int type)
{
	LuaSocket *ls = (LuaSocket *)lua_newuserdata(L, sizeof(LuaSocket));
	ls->closed = 1;
	luaL_setmetatable(L, loski_SocketClasses[type]);
	return ls;
}

/* socket = network.socket(type) */
static int net_socket (lua_State *L)
{
	static const char *const types[] = {"listen", "connection", "datagram", NULL};
	loski_NetState *net = tonet(L);
	int type = luaL_checkoption(L, 1, NULL, types);
	LuaSocket *ls = newsock(L, type);
	int res = loski_createsocket(net, &ls->socket, type);
	if (res == 0) ls->closed = 0;
	return pushsockres(L, 1, res);
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
	int res = loski_closesocket(net, &ls->socket);
	if (res == 0) ls->closed = 1;  /* mark socket as closed */
	return pushsockres(L, 0, res);
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
	loski_Address *na = loski_toaddress(L, idx);
	luaL_argcheck(L, na, idx, "address expected");
	return na;
}

/* succ [, errmsg] = socket:bind(address) */
static int sck_bind (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	const loski_Address *addr = checkaddress(L, 2);
	int res = loski_bindsocket(net, socket, addr);
	return pushsockres(L, 0, res);
}


/* address [, errmsg] = socket:getaddress(address [, site]) */
static int sck_getaddress (lua_State *L)
{
	static const char *const sites[] = {"local", "remote", NULL};
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address *addr = checkaddress(L, 2);
	int site = luaL_checkoption(L, 3, "local", sites);
	int res = loski_socketaddress(net, socket, addr, site);
	if (res == 0) lua_pushvalue(L, 2);
	return pushsockres(L, 1, res);
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
	int res = loski_getsocketoption(net, socket, opt, &val);
	if (res == 0) lua_pushboolean(L, val);
	return pushsockres(L, 1, res);
}

static int cnt_getoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val;
	int res = loski_getsocketoption(net, socket, opt, &val);
	if (res == 0) {
		if (opt == LOSKI_SOCKOPT_LINGER) lua_pushinteger(L, val);
		else                             lua_pushboolean(L, val);
	}
	return pushsockres(L, 1, res);
}

static int dgm_getoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val;
	int res = loski_getsocketoption(net, socket, opt, &val);
	if (res == 0) lua_pushboolean(L, val);
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:setoption(name, value) */
static int lst_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val = lua_toboolean(L, 3);
	int res = loski_setsocketoption(net, socket, opt, val);
	return pushsockres(L, 0, res);
}

static int cnt_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val = (opt == LOSKI_SOCKOPT_LINGER) ? luaL_checkint(L, 3)
	                                        : lua_toboolean(L, 3);
	int res = loski_setsocketoption(net, socket, opt, val);
	return pushsockres(L, 0, res);
}

static int dgm_setoption (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val = lua_toboolean(L, 3);
	int res = loski_setsocketoption(net, socket, opt, val);
	return pushsockres(L, 0, res);
}


/* succ [, errmsg] = socket:connect(address) */
static int str_connect (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	const loski_Address *addr = checkaddress(L, 2);
	int res = loski_connectsocket(net, socket, addr);
	return pushsockres(L, 0, res);
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
	int res;
	if (start < 1) start = 1;
	if (end > sz) end = sz;
	sz = end - start + 1;
	data += start - 1;
	res = loski_sendtosocket(net, socket, data, sz, &sent, addr);
	lua_pushinteger(L, sent);
	return pushsockres(L, 1, res);
}

/* sent [, errmsg] = socket:send(data [, i [, j]]) */
static int cnt_send (lua_State *L)
{
	return senddata(L, LOSKI_CONNSOCKET, NULL);
}

/* sent [, errmsg] = socket:send(data [, i [, j [, address]]]) */
static int dgm_send (lua_State *L)
{
	return senddata(L, LOSKI_DGRMSOCKET, loski_toaddress(L, 5));
}


static int recvdata(lua_State *L, int cls, loski_Address *addr)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, cls);
	size_t len, sz = (size_t)luaL_checkinteger(L, 2);
	luaL_Buffer lbuf;
	char *buf;
	int res;
	luaL_buffinit(L, &lbuf);
	buf = luaL_prepbuffsize(&lbuf, sz);  /* prepare buffer to read whole block */
	res = loski_recvfromsocket(net, socket, buf, sz, &len, addr);
	if (res == 0) {
		luaL_addsize(&lbuf, len);
		luaL_pushresult(&lbuf);  /* close buffer */
	}
	return pushsockres(L, 1, res);
}

/* data [, errmsg] = socket:receive(size) */
static int cnt_receive (lua_State *L)
{
	return recvdata(L, LOSKI_CONNSOCKET, NULL);
}

/* data [, errmsg] = socket:receive(size [, address]) */
static int dgm_receive (lua_State *L)
{
	return recvdata(L, LOSKI_DGRMSOCKET, loski_toaddress(L, 3));
}


/* succ [, errmsg] = socket:shutdown([mode]) */
static int cnt_shutdown (lua_State *L)
{
	static const char *const waynames[] = {"send", "receive", "both", NULL};
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	int way = luaL_checkoption(L, 2, "both", waynames);
	int res = loski_shutdownsocket(net, socket, way);
	return pushsockres(L, 0, res);
}


/* socket [, errmsg] = socket:accept([address]) */
static int lst_accept (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_Address *addr = loski_toaddress(L, 2);
	LuaSocket *ls = newsock(L, LOSKI_CONNSOCKET);
	int res = loski_acceptsocket(net, socket, &ls->socket, addr);
	if (res == 0) ls->closed = 0;
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:listen([backlog]) */
static int lst_listen (lua_State *L)
{
	loski_NetState *net = tonet(L);
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	int backlog = luaL_optint(L, 2, 32);
	int res = loski_listensocket(net, socket, backlog);
	return pushsockres(L, 0, res);
}


static const luaL_Reg sck[] = {
	{"__gc",        sck_gc},
	{"__tostring",  sck_tostring},
	{"close",       sck_close},
	{"bind",        sck_bind},
	{"getaddress",  sck_getaddress},
	{NULL,          NULL}
};

static const luaL_Reg lst[] = {
	{"getoption",   lst_getoption},
	{"setoption",   lst_setoption},
	{"accept",      lst_accept},
	{"listen",      lst_listen},
	{NULL,          NULL}
};

static const luaL_Reg str[] = {
	{"connect",     str_connect},
	{NULL,          NULL}
};

static const luaL_Reg cnt[] = {
	{"getoption",   cnt_getoption},
	{"setoption",   cnt_setoption},
	{"send",        cnt_send},
	{"receive",     cnt_receive},
	{"shutdown",    cnt_shutdown},
	{NULL,          NULL}
};

static const luaL_Reg dgm[] = {
	{"getoption",   dgm_getoption},
	{"setoption",   dgm_setoption},
	{"send",        dgm_send},
	{"receive",     dgm_receive},
	{NULL,          NULL}
};

static const luaL_Reg lib[] = {
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
#define UPS	1
#else
#define pushsentinel(L)	((void)L)
#define UPS	0
#endif
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_BASESOCKET], NULL, sck, UPS);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP listening socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_LSTNSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], lst, UPS);
	lua_pop(L, 1);  /* remove new class */
	/* create streaming socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_STRMSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], str, UPS);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP connection socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_CONNSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], cnt, UPS);
	lua_pop(L, 1);  /* remove new class */
	/* create UDP socket class */
	pushsentinel(L);
	luaL_newclass(L, loski_SocketClasses[LOSKI_DGRMSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], dgm, UPS);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, UPS);
	return 1;
}


LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls = ((LuaSocket *)luaL_testinstance(L, idx, loski_SocketClasses[cls]));
	return (ls != NULL) && (!ls->closed);
}

LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls;
	if (!loski_issocket(L, idx, cls)) return NULL;
	ls = ((LuaSocket *)lua_touserdata(L, idx));
	return &ls->socket;
}
