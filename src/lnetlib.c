#include "lnetlib.h"
#include "loskiaux.h"

#include <string.h>


#define pushaddrres(L,n,e) luaL_pushresults(L,n,e,loski_addresserror)
#define pushsockres(L,n,e) luaL_pushresults(L,n,e,loski_socketerror)


struct SocketOptionInfo {
	const char *name;
	loski_SocketOption option;
};

static int checksockopt (lua_State *L, int narg, struct SocketOptionInfo opts[]) {
	const char *name = luaL_checkstring(L, narg);
	int i;
	for (i=0; opts[i].name; i++)
		if (strcmp(opts[i].name, name) == 0)
			return opts[i].option;
	return luaL_argerror(L, narg,
	                     lua_pushfstring(L, "invalid option " LUA_QS, name));
}

static int checkaddress (loski_NetDriver *drv, lua_State *L, int nh, int np,
                         loski_Address *address) {
	const char *host = luaL_checkstring(L, nh);
	int port = luaL_checkint(L, np);
	luaL_argcheck(L, port >= 0 && port < 65536, 3, "invalid port value");
	return loski_resolveaddress(drv, address, host, (unsigned short)port);
}

static int pushaddress (loski_NetDriver *drv, lua_State *L,
                        const loski_Address *address) {
	const char *host;
	unsigned short port;
	int res = loski_extractaddress(drv, address, &host, &port);
	if (res == 0) {
		lua_pushstring(L, host);
		lua_pushnumber(L, port);
		return 2;
	}
	return pushaddrres(L, 0, res);
}


typedef struct LuaSocket {
	loski_Socket socket;
	int closed;
} LuaSocket;


#define tolsock(L,c)	((LuaSocket *)luaL_checkinstance(L, 1, loski_SocketClasses[c]))


static int sck_tostring (lua_State *L)
{
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	if (ls->closed)
		lua_pushliteral(L, "socket (closed)");
	else
		lua_pushfstring(L, "socket ("LOSKI_SOCKETSTRING")", ls->socket);
	return 1;
}


static loski_Socket *tosock (lua_State *L, int cls) {
	LuaSocket *ls = tolsock(L, cls);
	if (ls->closed)
		luaL_error(L, "attempt to use a closed socket");
	return &ls->socket;
}


static int aux_close (loski_NetDriver *drv, lua_State *L, LuaSocket *ls)
{
	int res = loski_closesocket(drv, &ls->socket);
	if (res == 0) ls->closed = 1;  /* mark socket as closed */
	return pushsockres(L, 0, res);
}


static int sck_close (lua_State *L)
{
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	tosock(L, LOSKI_BASESOCKET);  /* make sure argument is an open socket */
	return aux_close(drv, L, ls);
}


static int sck_gc (lua_State *L)
{
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	if (!ls->closed) aux_close(drv, L, ls);
	return 0;
}


static LuaSocket *newsock (lua_State *L, int type) {
	LuaSocket *ls = (LuaSocket *)lua_newuserdata(L, sizeof(LuaSocket));
	ls->closed = 1;
	luaL_setmetatable(L, loski_SocketClasses[type]);
	return ls;
}



/* socket = network.socket(type) */
static int net_socket (lua_State *L) {
	static const char *const types[] = {"listen", "connection", "datagram", NULL};
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	int type = luaL_checkoption(L, 1, NULL, types);
	LuaSocket *ls = newsock(L, type);
	int res = loski_createsocket(drv, &ls->socket, type);
	if (res == 0) ls->closed = 0;
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:bind(host, port) */
static int sck_bind (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address address;
	int res = checkaddress(drv, L, 2, 3, &address);
	if (res != 0) return pushaddrres(L, 0, res);
	res = loski_bindsocket(drv, socket, &address);
	return pushsockres(L, 0, res);
}


/* host, port = socket:getaddress([site]) */
static int sck_getaddress (lua_State *L) {
	static const char *const sites[] = {"local", "remote", NULL};
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address address;
	int site = luaL_checkoption(L, 2, "local", sites);
	int res = loski_socketaddress(drv, socket, &address, site);
	if (res == 0) return pushaddress(drv, L, &address);
	return pushsockres(L, 0, res);
}


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


/* value [, errmsg] = socket:getoption(name) */
static int lst_getoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val;
	int res = loski_getsocketoption(drv, socket, opt, &val);
	if (res == 0) lua_pushboolean(L, val);
	return pushsockres(L, 1, res);
}

static int cnt_getoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val;
	int res = loski_getsocketoption(drv, socket, opt, &val);
	if (res == 0) {
		if (opt == LOSKI_SOCKOPT_LINGER) lua_pushinteger(L, val);
		else                             lua_pushboolean(L, val);
	}
	return pushsockres(L, 1, res);
}

static int dgm_getoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val;
	int res = loski_getsocketoption(drv, socket, opt, &val);
	if (res == 0) lua_pushboolean(L, val);
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:setoption(name, value) */
static int lst_setoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, lst_opts);
	int val = lua_toboolean(L, 3);
	int res = loski_setsocketoption(drv, socket, opt, val);
	return pushsockres(L, 0, res);
}

static int cnt_setoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, cnt_opts);
	int val = (opt == LOSKI_SOCKOPT_LINGER) ? luaL_checkint(L, 3)
	                                        : lua_toboolean(L, 3);
	int res = loski_setsocketoption(drv, socket, opt, val);
	return pushsockres(L, 0, res);
}

static int dgm_setoption (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	loski_SocketOption opt = checksockopt(L, 2, dgm_opts);
	int val = lua_toboolean(L, 3);
	int res = loski_setsocketoption(drv, socket, opt, val);
	return pushsockres(L, 0, res);
}


/* succ [, errmsg] = socket:connect(host, port) */
static int str_connect (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	loski_Address address;
	int res = checkaddress(drv, L, 2, 3, &address);
	if (res != 0) return pushaddrres(L, 0, res);
	res = loski_connectsocket(drv, socket, &address);
	return pushsockres(L, 0, res);
}


static size_t posrelat (ptrdiff_t pos, size_t len) {
	if (pos >= 0) return (size_t)pos;
	else if (0u - (size_t)pos > len) return 0;
	else return len - ((size_t)-pos) + 1;
}

static const char *checkstream(lua_State *L,
                               int dataidx,
                               int startidx,
                               int endidx,
                               size_t *sz) {
	const char *data = luaL_checklstring(L, dataidx, sz);
	size_t start = posrelat(luaL_optinteger(L, startidx, 1), *sz);
	size_t end = posrelat(luaL_optinteger(L, endidx, -1), *sz);
	if (start < 1) start = 1;
	if (end > *sz) end = *sz;
	*sz = end - start + 1;
	return data + start - 1;
}

/* sent [, errmsg] = socket:send(data [, i [, j]]) */
static int cnt_send (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	size_t sz, sent;
	const char *data = checkstream(L, 2, 3, 4, &sz);
	int res = loski_sendtosocket(drv, socket, data, sz, &sent, NULL);
	lua_pushinteger(L, sent);
	return pushsockres(L, 1, res);
}

/* sent [, errmsg] = socket:send(data [, i [, j [, host, port]]]) */
static int dgm_send (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	size_t sz, sent;
	const char *data = checkstream(L, 2, 3, 4, &sz);
	int res;
	if (!lua_isnoneornil(L, 5)) {
		loski_Address address;
		res = checkaddress(drv, L, 5, 6, &address);
		if (res != 0) return pushaddrres(L, 0, res);
		res = loski_sendtosocket(drv, socket, data, sz, &sent, &address);
	} else {
		res = loski_sendtosocket(drv, socket, data, sz, &sent, NULL);
	}
	lua_pushinteger(L, sent);
	return pushsockres(L, 1, res);
}


static char *newbuffer(lua_State *L, int szidx, luaL_Buffer *lbuf, size_t *sz) {
	*sz = (size_t)luaL_checkinteger(L, szidx);
	luaL_buffinit(L, lbuf);
	return luaL_prepbuffsize(lbuf, *sz);  /* prepare buffer to read whole block */
}

/* data [, errmsg] = socket:receive(size) */
static int cnt_receive (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	luaL_Buffer lbuf;
	size_t sz, len;
	char *buf = newbuffer(L, 2, &lbuf, &sz);
	int res = loski_recvfromsocket(drv, socket, buf, sz, &len, NULL);
	if (res == 0) {
		luaL_addsize(&lbuf, len);
		luaL_pushresult(&lbuf);  /* close buffer */
	}
	return pushsockres(L, 1, res);
}

/* data [, errmsg] = socket:receive(size [, getfrom]) */
static int dgm_receive (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_DGRMSOCKET);
	luaL_Buffer lbuf;
	size_t sz, len;
	char *buf = newbuffer(L, 2, &lbuf, &sz);
	int res, nr = 1;
	if (lua_toboolean(L, 3)) {
		loski_Address address;
		res = loski_recvfromsocket(drv, socket, buf, sz, &len, &address);
		if (res == 0) {
			luaL_addsize(&lbuf, len);
			luaL_pushresult(&lbuf);  /* close buffer */
			nr += pushaddress(drv, L, &address);
		}
	} else {
		res = loski_recvfromsocket(drv, socket, buf, sz, &len, NULL);
		if (res == 0) {
			luaL_addsize(&lbuf, len);
			luaL_pushresult(&lbuf);  /* close buffer */
		}
	}
	return pushsockres(L, nr, res);
}


/* succ [, errmsg] = socket:shutdown([mode]) */
static int cnt_shutdown (lua_State *L) {
	static const char *const waynames[] = {"send", "receive", "both", NULL};
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_CONNSOCKET);
	int way = luaL_checkoption(L, 2, "both", waynames);
	int res = loski_shutdownsocket(drv, socket, way);
	return pushsockres(L, 0, res);
}


/* socket [, errmsg] = socket:accept([getfrom]) */
static int lst_accept (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	LuaSocket *ls = newsock(L, LOSKI_CONNSOCKET);
	int res, nr = 1;
	if (lua_toboolean(L, 2)) {
		loski_Address address;
		res = loski_acceptsocket(drv, socket, &ls->socket, &address);
		if (res == 0) nr += pushaddress(drv, L, &address);
	} else {
		res = loski_acceptsocket(drv, socket, &ls->socket, NULL);
	}
	if (res == 0) ls->closed = 0;
	return pushsockres(L, nr, res);
}


/* succ [, errmsg] = socket:listen([backlog]) */
static int lst_listen (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	int backlog = luaL_optint(L, 2, 32);
	int res = loski_listensocket(drv, socket, backlog);
	return pushsockres(L, 0, res);
}


static int net_sentinel (lua_State *L) {
	loski_NetDriver *drv = (loski_NetDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_closenetwork(drv);
	return 0;
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

LUAMOD_API int luaopen_network (lua_State *L)
{
	/* create sentinel */
	loski_NetDriver *drv = (loski_NetDriver *)luaL_newsentinel(L, sizeof(loski_NetDriver), net_sentinel);
	/* initialize library */
	if (loski_opennetwork(drv) != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
	/* create abstract base socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_BASESOCKET], NULL, sck, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP listening socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_LSTNSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], lst, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create streaming socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_STRMSOCKET],
	                 loski_SocketClasses[LOSKI_BASESOCKET], str, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP connection socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_CONNSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], cnt, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create UDP socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_DGRMSOCKET],
	                 loski_SocketClasses[LOSKI_STRMSOCKET], dgm, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	return 1;
}

LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls) {
	LuaSocket *ls = ((LuaSocket *)luaL_testinstance(L, idx, loski_SocketClasses[cls]));
	return (ls != NULL) && (!ls->closed);
}

LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls) {
	LuaSocket *ls;
	if (!loski_issocket(L, idx, cls)) return NULL;
	ls = ((LuaSocket *)lua_touserdata(L, idx));
	return &ls->socket;
}
