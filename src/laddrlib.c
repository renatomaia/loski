#include "addrlib.h"
#include "loskiaux.h"

#include <string.h>
#include <ctype.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"network.Address"

#define toaddr(L)	((loski_Address *)luaL_checkudata(L, 1, LOSKI_NETADDRCLS))

static loski_Address *newaddr (lua_State *L) {
	loski_Address *na = (loski_Address *)lua_newuserdata(L, sizeof(loski_Address));
	luaL_setmetatable(L, LOSKI_NETADDRCLS);
	return na;
}

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

static void pushtype (lua_State *L, loski_Address *na){
	loski_AddressType type = loski_addrtype(na);
	switch (type) {
		case LOSKI_ADDRTYPE_IPV4: lua_pushliteral(L, "ipv4"); break;
		case LOSKI_ADDRTYPE_IPV6: lua_pushliteral(L, "ipv6"); break;
		default: luaL_argerror(L, 1, "unsupported address type");
	}
}


/* address = address.create(data [, port [, format]]) */
static int addr_create (lua_State *L) {
	static const char *const formats[] = { "uri", "literal", "bytes", NULL };
	int format = luaL_checkoption(L, 3, "uri", formats);
	size_t sz;
	const char *data = luaL_checklstring(L, 1, &sz);
	loski_AddressType type;
	loski_AddressPort port = 0;
	if (format == 0) {  /* format == uri */
		const char *c, *e = data + sz;
		luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "port must be nil");
		if (data[0] == '[') {
			c = strchr(++data, ']');
			luaL_argcheck(L, c && c[1] == ':', 1, "invalid address");
			sz = c - data;
			type = LOSKI_ADDRTYPE_IPV6;
			++c;
		} else {
			c = strchr(data, ':');
			luaL_argcheck(L, c, 1, "invalid address");
			sz = c - data;
			type = LOSKI_ADDRTYPE_IPV4;
		}
		luaL_argcheck(L, str2port(c+1, &port) == e, 1, "invalid port");
	} else {
		lua_Integer n = luaL_checkinteger(L, 2);
		luaL_argcheck(L, 0 <= n && n <= LOSKI_ADDRMAXPORT, 2, "invalid port");
		port = (loski_AddressPort)n;
		if (format == 2) {  /* format == bytes */
			switch (sz) {
				case LOSKI_ADDRSIZE_IPV4: type = LOSKI_ADDRTYPE_IPV4; break;
				case LOSKI_ADDRSIZE_IPV6: type = LOSKI_ADDRTYPE_IPV6; break;
				default: luaL_argerror(L, 1, "invalid address"); type = 0;
			}
			luaL_argcheck(L, loski_writeaddrbytes(newaddr(L), type, data, port), 1,
			                 "invalid address");
			return 1;
		}
		/* format == literal */
		type = strchr(data, ':') ? LOSKI_ADDRTYPE_IPV6 : LOSKI_ADDRTYPE_IPV4;
	}
	luaL_argcheck(L, loski_writeaddrliteral(newaddr(L), type, data, sz, port), 1,
	                 "invalid address");
	return 1;
}

/* type = address.type(addr) */
static int addr_type (lua_State *L) {
	loski_Address *na = (loski_Address *)luaL_testudata(L, 1, LOSKI_NETADDRCLS);
	if (na) pushtype(L, na);
	else lua_pushnil(L);
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
	loski_Address *na = toaddr(L);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			lua_pushinteger(L, loski_addrport(na));
		} break;
		case 1: {  /* bytes */
			size_t sz;
			const char *s = loski_addrbytes(na, &sz);
			if (s) lua_pushlstring(L, s, sz);
			else lua_pushnil(L);
		} break;
		case 2: {  /* literal */
			char b[LOSKI_ADDRMAXLITERAL];
			const char *s = loski_addrliteral(na, b);
			if (s) lua_pushstring(L, s);
			else lua_pushnil(L);
		} break;
		case 3: {  /* type */
			pushtype(L, na);
		} break;
	}
	return 1;
}

/* uri = tostring(address) */
static int addr_tostring (lua_State *L)
{
	loski_Address *na = toaddr(L);
	loski_AddressType type = loski_addrtype(na);
	loski_AddressPort port = loski_addrport(na);
	char b[LOSKI_ADDRMAXLITERAL];
	const char *s = loski_addrliteral(na, b);
	if (s) lua_pushfstring(L, type==LOSKI_ADDRTYPE_IPV6?"[%s]:%d":"%s:%d",s,port);
	else lua_pushliteral(L, "<corrupted data>");
	return 1;
}


static const luaL_Reg lib[] = {
	{"create", addr_create},
	{"type", addr_type},
	{NULL, NULL}
};

static const luaL_Reg mth[] = {
	{"__index", addr_index},
	{"__tostring", addr_tostring},
	{NULL, NULL}
};

LUAMOD_API int luaopen_network_address (lua_State *L)
{
	/* create object metatable */
	luaL_newclass(L, LOSKI_NETADDRCLS, NULL, mth, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	return 1;
}
