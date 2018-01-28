#include "luaosi/levtlib.h"


#ifdef LOSKI_DISABLE_EVTDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)	((loski_EventDriver *)lua_touserdata(L, \
                                        lua_upvalueindex(DRVUPV)))
#endif


static void getsource(lua_State *L, loski_EventSource *source)
{
	loski_EventSourceConv tosource = loski_geteventsourceconv(L);
	if (tosource == NULL || !tosource(L, -1, source))
		luaL_error(L, "invalid watchable object");
}

#define pushsrcid(L,D,S)	(lua_pushinteger(L, loskiE_getsourceid(D,S)))


/* watcher [, errmsg] = event.watcher() */
static int evt_watcher (lua_State *L)
{
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = loski_newwatcher(L);
	loski_ErrorCode err = loskiE_initwatcher(drv, watcher);
	if (!err) loski_enablewatcher(L);
	return loskiL_doresults(L, 1, err);
}


#define tolwatcher(L)	((LuaWatcher *)luaL_checkudata(L, 1, LOSKI_WATCHERCLS))

/* string = tostring(watcher) */
static int ew_tostring (lua_State *L)
{
	LuaWatcher *lw = tolwatcher(L);
	if (lw->closed)
		lua_pushliteral(L, "event watcher (closed)");
	else
		lua_pushfstring(L, "event watcher (%p)", lw->watcher);
	return 1;
}


static int endwatcher (loski_EventDriver *drv, lua_State *L, LuaWatcher *lw)
{
	loski_ErrorCode err = loskiE_endwatcher(drv, &lw->watcher);
	if (!err) {
		lw->closed = 1;  /* mark watcher as closed */
		lua_pushnil(L);
		lua_setuservalue(L, 1);
	}
	return loskiL_doresults(L, 0, err);
}

/* getmetatable(watcher).__gc(watcher) */
static int ew_gc(lua_State *L)
{
	loski_EventDriver *drv = todrv(L);
	LuaWatcher *lw = tolwatcher(L);
	if (!lw->closed) endwatcher(drv, L, lw);
	return 0;
}


static loski_EventWatcher *towatcher (lua_State *L) {
	LuaWatcher *lw = tolwatcher(L);
	if (lw->closed) luaL_error(L, "attempt to use a closed event watcher");
	return &lw->watcher;
}

/* succ [, errmsg] = watcher:close() */
static int ew_close(lua_State *L)
{
	loski_EventDriver *drv = todrv(L);
	LuaWatcher *lw = tolwatcher(L);
	towatcher(L);  /* make sure argument is a valid watcher */
	return endwatcher(drv, L, lw);
}


/* succ [, errmsg] = watcher:set(source [, events]) */
static int ew_set (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	loski_EventSource source;
	loski_EventFlags evtflags = LOSKI_EVTFLAGS_NONE;
	const char *events = luaL_optstring(L, 3, "");
	loski_ErrorCode err;
	for (; *events; ++events) switch (*events) {
		case 'r': evtflags |= LOSKI_EVTFLAGS_INPUT; break;
		case 'w': evtflags |= LOSKI_EVTFLAGS_OUTPUT; break;
		default: return luaL_error(L, "unknown event (got '%c')", *events);
	}
	lua_settop(L, 2);
	getsource(L, &source);
	err = loskiE_setwatch(drv, watcher, &source, evtflags);
	if (!err) {
		lua_getuservalue(L, 1);
		pushsrcid(L, drv, &source);
		if (evtflags == LOSKI_EVTFLAGS_NONE) lua_pushnil(L);
		else                                 lua_pushvalue(L, 2);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}
	return loskiL_doresults(L, 0, err);
}


/* map [, errmsg] = watcher:wait([timeout]) */
static int ew_wait (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	lua_Number timeout = luaL_optnumber(L, 2, -1);
	loski_ErrorCode err = loskiE_waitevent(drv, watcher, timeout);
	lua_settop(L, 1);
	if (!err) {
		size_t i = 0;
		loski_EventSource source;
		loski_EventFlags evtflags;
		lua_getuservalue(L, 1);  /* places source registry at index 2 */
		lua_newtable(L);  /* creates result table at index 3 */
		while (loskiE_nextevent(drv, watcher, &i, &source, &evtflags)) {
			pushsrcid(L, drv, &source);
			lua_gettable(L, 2);  /* get source from registry (index 2) */
			switch (evtflags) {
				case LOSKI_EVTFLAGS_INPUT:
					lua_pushliteral(L, "r");
					break;
				case LOSKI_EVTFLAGS_OUTPUT:
					lua_pushliteral(L, "w");
					break;
				case LOSKI_EVTFLAGS_INPUT|LOSKI_EVTFLAGS_OUTPUT:
					lua_pushliteral(L, "rw");
					break;
				default:
					lua_pushnil(L);
					break;
			}
			lua_settable(L, 3);  /* places into result table (index 3) */
		}
	}
	return loskiL_doresults(L, 1, err);
}


#ifndef LOSKI_DISABLE_EVTDRV
static int lfreedrv (lua_State *L) {
	loski_EventDriver *drv = (loski_EventDriver *)lua_touserdata(L, 1);
	loskiE_freedrv(drv);
	return 0;
}
#endif

static const luaL_Reg wch[] = {
	{"__tostring", ew_tostring},
	{"__gc", ew_gc},
	{"set", ew_set},
	{"wait", ew_wait},
	{"close", ew_close},
	{NULL, NULL}
};

static const luaL_Reg lib[] = {
	{"watcher", evt_watcher},
	{NULL, NULL}
};

LUAMOD_API int luaopen_event(lua_State *L)
{
#ifndef LOSKI_DISABLE_EVTDRV
	/* create sentinel */
	loski_EventDriver *drv;
	loski_ErrorCode err;
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (loski_EventDriver *)luaL_newsentinel(L, sizeof(loski_EventDriver),
	                                               lfreedrv);
	/* initialize library */
	err = loskiE_initdrv(drv);
	if (err) {
		luaL_cancelsentinel(L);
		loskiL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, LOSKI_WATCHERCLS, NULL, wch, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);

#ifdef LOSKI_ENABLE_WATCHEREVENTS
	loski_seteventsourceconv(L, LOSKI_WATCHERCLS, loskiE_watcher2evtsrc);
#endif
#ifdef LOSKI_ENABLE_LUAFILEEVENTS
	loski_seteventsourceconv(L, LUA_FILEHANDLE, loskiE_luafile2evtsrc);
#endif
	return 1;
}
