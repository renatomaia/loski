#include "laddrlib.h"
#include "loskiaux.h"

#include <string.h>
#include <ctype.h>


#define toaddr(L)	((loski_Address *)luaL_checkudata(L, 1, LOSKI_NETADDRCLS))
#define optaddr(L,i)	((loski_Address *)luaL_testudata(L, i, LOSKI_NETADDRCLS))

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
	loski_AddressType type = loski_getaddrtype(na);
	switch (type) {
		case LOSKI_ADDRTYPE_IPV4: lua_pushliteral(L, "ipv4"); break;
		case LOSKI_ADDRTYPE_IPV6: lua_pushliteral(L, "ipv6"); break;
		default: luaL_argerror(L, 1, "corrupted data");
	}
}

static loski_AddressPort int2port (lua_State *L, lua_Integer n, int idx){
	luaL_argcheck(L, 0 <= n && n <= LOSKI_ADDRMAXPORT, idx, "invalid port");
	return (loski_AddressPort)n;
}


/* address = address.create([data [, port [, format]]]) */
static int addr_create (lua_State *L) {
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
	luaL_argcheck(L, loski_initaddress(na, type), 1, "unsupported address");
	if (data) {
		if (binary) loski_setaddrbytes(na, data);
		else luaL_argcheck(L, loski_setaddrliteral(na, data, sz), 1,
		                      "invalid literal address");
		if (port) loski_setaddrport(na, port);
	}
	return 1;
}

/* type = address.type(addr) */
static int addr_type (lua_State *L) {
	loski_Address *na = optaddr(L, 1);
	if (na) pushtype(L, na);
	else lua_pushnil(L);
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
	loski_Address *na = toaddr(L);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			lua_pushinteger(L, loski_getaddrport(na));
		} break;
		case 1: {  /* binary */
			size_t sz;
			const char *s = loski_getaddrbytes(na, &sz);
			if (s) lua_pushlstring(L, s, sz);
			else lua_pushnil(L);
		} break;
		case 2: {  /* literal */
			char b[LOSKI_ADDRMAXLITERAL];
			const char *s = loski_getaddrliteral(na, b);
			if (s) lua_pushstring(L, s);
			else lua_pushnil(L);
		} break;
		case 3: {  /* type */
			pushtype(L, na);
		} break;
	}
	return 1;
}

/*
 * address.type = type
 * address.literal = literal
 * address.binary = binary
 * address.port = port
 */
static int addr_newindex (lua_State *L) {
	static const char *const fields[] = {"port","binary","literal","type",NULL};
	loski_Address *na = toaddr(L);
	int field = luaL_checkoption(L, 2, NULL, fields);
	switch (field) {
		case 0: {  /* port */
			loski_setaddrport(na, int2port(L, luaL_checkinteger(L, 3), 3));
		} break;
		case 1: {  /* binary */
			size_t sz, esz = 0;
			const char *data = luaL_checklstring(L, 3, &sz);
			loski_AddressType type = loski_getaddrtype(na);
			switch (type) {
				case LOSKI_ADDRTYPE_IPV4: esz = LOSKI_ADDRSIZE_IPV4; break;
				case LOSKI_ADDRTYPE_IPV6: esz = LOSKI_ADDRSIZE_IPV6; break;
				default: luaL_argerror(L, 1, "corrupted data");
			}
			luaL_argcheck(L, sz == esz, 3, "wrong byte size");
			loski_setaddrbytes(na, data);
		} break;
		case 2: {  /* literal */
			size_t sz;
			const char *data = luaL_checklstring(L, 3, &sz);
			luaL_argcheck(L, loski_setaddrliteral(na, data, sz), 3, "invalid value");
		} break;
		case 3: {  /* type */
			static const char *const types[] = {"ipv4","ipv6",NULL};
			int typeid = luaL_checkoption(L, 3, NULL, types);
			loski_AddressType type = loski_getaddrtype(na);
			loski_AddressType new = type;
			switch (typeid) {
				case 0: new = LOSKI_ADDRTYPE_IPV4; break;
				case 1: new = LOSKI_ADDRTYPE_IPV6; break;
			}
			if (new != type)
				luaL_argcheck(L, loski_initaddress(na, new), 3, "unsupported address");
		} break;
	}
	return 0;
}

/* addr1 == addr2 */
static int addr_eq (lua_State *L)
{
	loski_Address *a1 = optaddr(L, 1);
	loski_Address *a2 = optaddr(L, 2);
	if (a1 && a2 && (loski_getaddrtype(a1) == loski_getaddrtype(a2))
		           && (loski_getaddrport(a1) == loski_getaddrport(a2))) {
		size_t sz;
		const char *b = loski_getaddrbytes(a1, &sz);
		lua_pushboolean(L, memcmp(b, loski_getaddrbytes(a2, NULL), sz) == 0);
	}
	else lua_pushboolean(L, 0);
	return 1;
}

/* uri = tostring(address) */
static int addr_tostring (lua_State *L)
{
	loski_Address *na = toaddr(L);
	loski_AddressType type = loski_getaddrtype(na);
	loski_AddressPort port = loski_getaddrport(na);
	char b[LOSKI_ADDRMAXLITERAL];
	const char *s = loski_getaddrliteral(na, b);
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
	{"__newindex", addr_newindex},
	{"__eq", addr_eq},
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


LOSKILIB_API int loski_isaddress (lua_State *L, int idx)
{
	return luaL_testudata(L, idx, LOSKI_NETADDRCLS) != NULL;
}

LOSKILIB_API loski_Address *loski_toaddress (lua_State *L, int idx)
{
	if (!loski_isaddress(L, idx)) return NULL;
	return (loski_Address *)lua_touserdata(L, idx);
}
