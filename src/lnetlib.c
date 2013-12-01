#include "lnetlib.h"
#include "loskiaux.h"

#include <string.h>


#define pushaddrres(L,n,e) luaL_pushresults(L,n,e,loski_addresserror)
#define pushsockres(L,n,e) luaL_pushresults(L,n,e,loski_socketerror)


static int pushaddress(lua_State *L, const loski_Address *address) {
	const char *host;
	unsigned short port;
	int res = loski_extractaddress(address, &host, &port);
	if (res == 0) {
		lua_pushstring(L, host);
		lua_pushnumber(L, port);
		return 2;
	}
	return pushaddrres(L, 0, res);
}

static int checkaddress(lua_State *L, int nh, int np,
                        loski_Address *address) {
	const char *host = luaL_checkstring(L, nh);
	int port = luaL_checkint(L, np);
	luaL_argcheck(L, port >= 0 && port < 65536, 3, "invalid port value");
	return loski_resolveaddress(address, host, (unsigned short)port);
}


typedef struct LuaSocket {
	loski_Socket socket;
	int closed;
} LuaSocket;


static const char *const SocketTypes[] = {
	"datagram",
	"connection",
	"listen",
	NULL
};

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


static int aux_close (lua_State *L, LuaSocket *ls)
{
	int res = loski_closesocket(&ls->socket);
	if (res == 0) ls->closed = 1;  /* mark socket as closed */
	return pushsockres(L, 0, res);
}


static int sck_close (lua_State *L)
{
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	tosock(L, LOSKI_BASESOCKET);  /* make sure argument is an open socket */
	return aux_close(L, ls);
}


static int sck_gc (lua_State *L)
{
	LuaSocket *ls = tolsock(L, LOSKI_BASESOCKET);
	if (!ls->closed) aux_close(L, ls);
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
	int type = luaL_checkoption(L, 1, NULL, SocketTypes);
	LuaSocket *ls = newsock(L, type);
	int res = loski_createsocket(&ls->socket, type);
	if (res == 0) ls->closed = 0;
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:bind(host, port) */
static int sck_bind (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address address;
	int res = checkaddress(L, 2, 3, &address);
	if (res != 0) return pushaddrres(L, 0, res);
	res = loski_bindsocket(socket, &address);
	return pushsockres(L, 0, res);
}


/* host, port = socket:getaddress([site]) */
static int sck_getaddress (lua_State *L) {
	static const char *const sitenames[] = {"remote", "local", NULL};
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	loski_Address address;
	int site = luaL_checkoption(L, 2, "remote", sitenames);
	int res = loski_socketaddress(socket, &address, site);
	if (res == 0) return pushaddress(L, &address);
	return pushsockres(L, 0, res);
}


static const char *const sck_opts[] = {
	/* any */
	"blocking",
	"reuseaddr",
	"dontroute",
	/* connection */
	"linger",
	"keepalive",
	"nodelay",
	/* datagram */
	"broadcast",
	NULL
};


/* value [, errmsg] = socket:getoption(name) */
static int sck_getoption (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	int opt = luaL_checkoption(L, 2, NULL, sck_opts);
	int val;
	int res = loski_getsocketoption(socket, opt, &val);
	switch (opt) {
		case LOSKI_SOCKOPT_LINGER:
			lua_pushinteger(L, val);
			break;
		default:
			lua_pushboolean(L, val);
			break;
	}
	return pushsockres(L, 1, res);
}


/* succ [, errmsg] = socket:setoption(name, value) */
static int sck_setoption (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	int opt = luaL_checkoption(L, 2, NULL, sck_opts);
	int val, res;
	switch (opt) {
		case LOSKI_SOCKOPT_LINGER:
			val = luaL_checkint(L, 3);
			break;
		default:
			luaL_checkany(L, 3);
			val = lua_toboolean(L, 3);
			break;
	}
	res = loski_setsocketoption(socket, opt, val);
	return pushsockres(L, 0, res);
}


/* succ [, errmsg] = socket:connect(host, port) */
static int sck_connect (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	loski_Address address;
	int res = checkaddress(L, 2, 3, &address);
	if (res != 0) return pushaddrres(L, 0, res);
	res = loski_connectsocket(socket, &address);
	return pushsockres(L, 0, res);
}


/* sent [, errmsg] = socket:send(data [, i [, j [, host, port]]]) */
static int udp_send (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	size_t sz, sent;
	const char *data = luaL_checklstring(L, 2, &sz);
	//int i = luaL_optinteger(L, 3, 1);
	//int j = luaL_optinteger(L, 4, sz);
	int res;
	if (!lua_isnoneornil(L, 5)) {
		loski_Address address;
		res = checkaddress(L, 5, 6, &address);
		if (res != 0) return pushaddrres(L, 0, res);
		res = loski_sendtosocket(socket, data, sz, &sent, &address);
	} else {
		res = loski_sendtosocket(socket, data, sz, &sent, NULL);
	}
	lua_pushinteger(L, sent);
	return pushsockres(L, 1, res);
}


/* data [, errmsg] = socket:receive(pattern [, getfrom]) */
static int udp_receive (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_STRMSOCKET);
	size_t len;  /* number of chars actually read */
	char *buf;
	luaL_Buffer lbuf;
	size_t sz = (size_t)luaL_checkinteger(L, 2);
	int res, nr = 1;
	luaL_buffinit(L, &lbuf);
	buf = luaL_prepbuffsize(&lbuf, sz);  /* prepare buffer to read whole block */
	if (lua_toboolean(L, 3)) {
		loski_Address address;
		res = loski_recvfromsocket(socket, buf, sz, &len, &address);
		if (res == 0) {
			luaL_addsize(&lbuf, len);
			luaL_pushresult(&lbuf);  /* close buffer */
			nr += pushaddress(L, &address);
		}
	} else {
		res = loski_recvfromsocket(socket, buf, sz, &len, NULL);
		if (res == 0) {
			luaL_addsize(&lbuf, len);
			luaL_pushresult(&lbuf);  /* close buffer */
		}
	}
	return pushsockres(L, nr, res);
}


//static int tcp_send (lua_State *L) {
//
//}
//
//
//static int tcp_receive (lua_State *L) {
//
//}


/* succ [, errmsg] = socket:shutdown([mode]) */
static int tcp_shutdown (lua_State *L) {
	static const char *const waynames[] = {"receive", "send", "both", NULL};
	loski_Socket *socket = tosock(L, LOSKI_BASESOCKET);
	int way = luaL_checkoption(L, 2, "both", waynames);
	int res = loski_shutdownsocket(socket, way);
	return pushsockres(L, 0, res);
}


/* socket [, errmsg] = socket:accept([getfrom]) */
static int tcp_accept (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	LuaSocket *ls = newsock(L, LOSKI_CONNSOCKET);
	int res, nr = 1;
	if (lua_toboolean(L, 2)) {
		loski_Address address;
		res = loski_acceptsocket(socket, &ls->socket, &address);
		if (res == 0) nr += pushaddress(L, &address);
	} else {
		res = loski_acceptsocket(socket, &ls->socket, NULL);
	}
	if (res == 0) ls->closed = 0;
	return pushsockres(L, nr, res);
}


/* succ [, errmsg] = socket:listen([backlog]) */
static int tcp_listen (lua_State *L) {
	loski_Socket *socket = tosock(L, LOSKI_LSTNSOCKET);
	int backlog = luaL_optint(L, 2, 32);
	int res = loski_listensocket(socket, backlog);
	return pushsockres(L, 0, res);
}


static int net_sentinel (lua_State *L) {
	loski_closenetwork();
	return 0;
}


static const luaL_Reg sck[] = {
	{"__gc",        sck_gc},
	{"__tostring",  sck_tostring},
	{"close",       sck_close},
	{"bind",        sck_bind},
	{"getaddress",  sck_getaddress},
	{"getoption",   sck_getoption},
	{"setoption",   sck_setoption},
	{NULL,          NULL}
};

static const luaL_Reg str[] = {
	{"connect",     sck_connect},
	{NULL,          NULL}
};

static const luaL_Reg udp[] = {
	{"send",        udp_send},
	{"receive",     udp_receive},
	{NULL,          NULL}
};

static const luaL_Reg tcp[] = {
	{"send",        udp_send},
	{"receive",     udp_receive},
	{"shutdown",    tcp_shutdown},
	{NULL,          NULL}
};

static const luaL_Reg lst[] = {
	{"accept",      tcp_accept},
	{"listen",      tcp_listen},
	{NULL,          NULL}
};

static const luaL_Reg lib[] = {
	{"socket", net_socket},
	{NULL, NULL}
};

LUAMOD_API int luaopen_network (lua_State *L)
{
	int res;
	/* create sentinel */
	luaL_newsentinel(L, net_sentinel);
	/* initialize library */
	res = loski_opennetwork();
	if (res != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library (%s)", strerror(res));
	}
	/* create abstract base socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, loski_SocketClasses[LOSKI_BASESOCKET], sck, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create abstract streamming socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newsubclass(L, loski_SocketClasses[LOSKI_BASESOCKET],
	                    loski_SocketClasses[LOSKI_STRMSOCKET], str, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create UDP socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newsubclass(L, loski_SocketClasses[LOSKI_STRMSOCKET],
	                    loski_SocketClasses[LOSKI_DGRMSOCKET], udp, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP connection socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newsubclass(L, loski_SocketClasses[LOSKI_STRMSOCKET],
	                    loski_SocketClasses[LOSKI_CONNSOCKET], tcp, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create TCP listening socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newsubclass(L, loski_SocketClasses[LOSKI_BASESOCKET],
	                    loski_SocketClasses[LOSKI_LSTNSOCKET], lst, 1);
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
	if (!loski_issocket(L, idx, cls)) return NULL;
	LuaSocket *ls = ((LuaSocket *)lua_touserdata(L, idx));
	return &ls->socket;
}
