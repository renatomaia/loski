#include "lnetlib.h"
#include "loskiaux.h"

#include <string.h>


#define pushaddrres(L,n,e) luaL_pushresults(L,n,e,loski_addresserror)

#define todrv(L)	((loski_NetAddrDriver *)lua_touserdata(L, lua_upvalueindex(1)))
#define toaddr(L)	((loski_Address *)luaL_checkudata(L, 1, LOSKI_NETADDRCLS))

static LuaSocket *newaddr (lua_State *L) {
	loski_Address *na = (loski_Address *)lua_newuserdata(L, sizeof(loski_Address));
	luaL_setmetatable(L, LOSKI_NETADDRCLS);
	return na;
}

static const char *str2port (const char *s, loski_PortNo *pn) {
	lua_Unsigned n = 0;
	if (!isdigit((unsigned char)*s)) return NULL;  /* no digit? */
	do {
		n = n * 10 + (*s - '0');
		if (n > LOSKI_ADDRMAXPORT) return NULL;  /* value too large */
		++s;
	} while (isdigit((unsigned char)*s));
	*pn = (loski_PortNo)n;
	return s;
}

static int pushtype (lua_State *L, loski_NetAddrDriver *drv, loski_Address *na){
	loski_AddressType type;
	res = loski_getaddrtype(drv, na, &type);
	if (res == 0) {
		switch (type) {
			case LOSKI_ADDRTYPE_IPV4: lua_pushliteral(L, "ipv4"); break;
			case LOSKI_ADDRTYPE_IPV6: lua_pushliteral(L, "ipv6"); break;
			default: luaL_argerror(L, 1, "unsupported address type");
		}
	}
	return res;
}


/* address = address.create(data [, port [, type]]) */
static int addr_create (lua_State *L) {
	static const char *const format[] = { "uri", "literal", "bytes", NULL };
	loski_NetAddrDriver *drv = todrv(L);
	int type = luaL_checkoption(L, 3, "uri", format);
	size_t sz;
	const char *data = luaL_checklstring(L, 1, &sz);
	loski_PortNo port = 0;
	loski_Address *na = newaddr(L);
	int res, raw;
	if (type == 0) { /* uri */
		const char *c, *e = data + sz;
		luaL_argcheck(L, lua_isnil(L, 2), 2, "port provided with URI defintion");
		if (data[0] == '[')) {
			c = strchr(++data, ']');
			luaL_argcheck(L, c && c[1] == ':', 1, "invalid URI address");
			sz = c - data;
			type = LOSKI_ADDRTYPE_IPV6;
			++c;
		} else {
			c = strchr(data, ':');
			luaL_argcheck(L, c, 1, "invalid URI address");
			sz = c - data;
			type = LOSKI_ADDRTYPE_IPV4;
		}
		luaL_argcheck(L, str2port(c+1, &port) == e, 2,
		                          "invalid port in URI address");
		raw = 0;
	} else {
		lua_Integer n = luaL_checkinteger(L, 3);
		luaL_argcheck(L, 0 <= n && n <= LOSKI_ADDRMAXPORT, 3, "invalid port value");
		port = (loski_PortNo)n;
		if (raw = type % 2) {
			switch (sz) {
				case LOSKI_ADDRSIZE_IPV4: type = LOSKI_ADDRTYPE_IPV4; break;
				case LOSKI_ADDRSIZE_IPV6: type = LOSKI_ADDRTYPE_IPV6; break;
				default: luaL_argerror(L, 1, "invalid address byte data");
			}
		} else {
			type = strchr(data, ':') ? LOSKI_ADDRTYPE_IPV6 : LOSKI_ADDRTYPE_IPV4;
		}
	}
	if (raw) {
		res = loski_writeaddrrawdata(drv, na, type, data, port);
	} else {
		res = loski_writeaddrliteral(drv, na, type, data, sz, port);
	}
	return pushaddrres(L, 1, res);
}

/* type = address.type(addr) */
static int addr_type (lua_State *L) {
	loski_NetAddrDriver *drv = todrv(L);
	loski_Address *na = (loski_Address *)luaL_testudata(L, idx, LOSKI_NETADDRCLS));
	if (na) {
		int res = pushtype(L, drv, na);
		return pushaddrres(L, 1, res);
	}
	lua_pushnil(L);
	return 1;
}

/*
 * type = address.type
 * literal = address.literal
 * bytes = address.bytes
 * port = address.port
 */
static int addr_index (lua_State *L) {
	static const char *const fields[] = {"port","bytes","literal","type",NULL};
	loski_NetAddrDriver *drv = todrv(L);
	loski_Address *na = toaddr(L);
	int field = luaL_checkoption(L, 3, NULL, fields);
	int res;
	switch (field) {
		case 0: {  /* port */
			loski_PortNo port;
			res = loski_readaddrport(drv, na, &port);
			if (res == 0) lua_pushstring(L, data);
		} break;
		case 1: {  /* bytes */
			const char *data;
			size_t sz;
			res = loski_readaddrbytes(drv, na, &data, &sz);
			if (res == 0) lua_pushlstring(L, data, sz);
		} break;
		case 2: {  /* literal */
			char *data[LOSKI_ADDRMAX_LITERAL];
			res = loski_readaddrliteral(drv, na, data);
			if (res == 0) lua_pushstring(L, data);
		} break;
		case 3: {  /* type */
			res = pushtype(L, drv, na);
		} break;
	}
	return pushaddrres(L, 1, res);
}

/* uri = tostring(address) */
static int addr_tostring (lua_State *L)
{
	loski_NetAddrDriver *drv = todrv(L);
	loski_Address *na = toaddr(L);
	char *data[LOSKI_ADDRMAX_URI];
	res = loski_readaddruri(drv, na, data);
	if (res == 0) lua_pushstring(L, data);
	return pushaddrres(L, 1, res);
}


static int addr_sentinel (lua_State *L) {
	loski_NetAddrDriver *drv = todrv(L);
	loski_closenetaddr(drv);
	return 0;
}


static const luaL_Reg lib[] = {
	{"create", addr_create},
	{"type", addr_type},
	{NULL, NULL}
};

static const luaL_Reg mth[] = {
	{"__index", addr_index},
	{"__tostring", addr_uri},
	{NULL, NULL}
};

LUAMOD_API int luaopen_network_address (lua_State *L)
{
	/* create sentinel */
	loski_NetAddrDriver *drv = (loski_NetAddrDriver *)luaL_newsentinel(L,
		sizeof(loski_NetAddrDriver), addr_sentinel);
	/* initialize library */
	if (loski_opennetaddr(drv) != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
	/* create abstract base socket class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, LOSKI_NETADDRCLS, NULL, mth, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	return 1;
}
