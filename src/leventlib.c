#include "leventaux.h"


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

static void pushregkey(lua_State *L, loski_EventSource *source) {
	loski_EventDriver *drv = todrv(L);
	lua_getuservalue(L, 1);
	lua_pushinteger(L, loskiE_getsourceid(drv, source));
}

static void savesource(lua_State *L, loski_EventSource *source) {
	pushregkey(L, source);
	lua_pushvalue(L, 2);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

static int knownsource(lua_State *L, loski_EventSource *source) {
	int found = 0;
	pushregkey(L, source);
	lua_gettable(L, -2);
	found = !lua_isnil(L, -1);
	lua_pop(L, 2);
	return found;
}

static void forgetsource(lua_State *L, loski_EventSource *source) {
	pushregkey(L, source);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

static void pushsource(lua_State *L, loski_EventSource *source) {
	pushregkey(L, source);
	lua_gettable(L, -2);
	lua_remove(L, -2);
}


/* watcher [, errmsg] = event.watcher() */
static int evt_watcher (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = loski_newwatcher(L);
	loski_ErrorCode err = loskiE_initwatcher(drv, watcher);
	if (!err) loski_enablewatcher (L, -1);
	return loskiL_doresults(L, 1, err);
}


#define tolwatcher(L)	((LuaWatcher *)luaL_checkudata(L, 1, LOSKI_WATCHERCLS))

/* string = tostring(watcher) */
static int ew_tostring (lua_State *L)
{
	LuaWatcher *p = tolwatcher(L);
	if (p->closed)
		lua_pushliteral(L, "event watcher (closed)");
	else
		lua_pushfstring(L, "event watcher (%p)", p);
	return 1;
}


static int endwatcher (loski_EventDriver *drv, lua_State *L, LuaWatcher *p)
{
	loski_ErrorCode err = loskiE_endwatcher(drv, &p->watcher);
	if (!err) {
		p->closed = 1;  /* mark watcher as closed */
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


static const char *const EventName[] = {
	"r",
	"w",
	NULL
};
static const loski_EventValue EventValue[] = {
	LOSKI_EVTVAL_INPUT,
	LOSKI_EVTVAL_OUTPUT
};

/* succ [, errmsg] = watcher:add(source [, events]) */
static int ew_add (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	loski_EventSource source;
	int optevt = luaL_checkoption(L, 3, "r", EventName);
	loski_ErrorCode err;
	lua_settop(L, 2);
	getsource(L, &source);
	err = loskiE_addwatch(drv, watcher, &source, EventValue[optevt]);
	if (!err) savesource(L, &source);
	return loskiL_doresults(L, 0, err);
}


/* succ [, errmsg] = watcher:remove(object [, event]) */
static int ew_remove (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	loski_EventSource source;
	int optevt = luaL_checkoption(L, 3, "r", EventName);
	lua_settop(L, 2);
	getsource(L, &source);
	if (knownsource(L, &source)) {
		loski_ErrorCode err = loskiE_delwatch(drv, watcher, &source,
		                                      EventValue[optevt]);
		if (!err) forgetsource(L, &source);
		return loskiL_doresults(L, 0, err);
	}
	lua_pushnil(L);
	lua_pushliteral(L, "not found");
	return 2;
}


/* map [, errmsg] = watcher:wait([timeout]) */
static int ew_wait (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	lua_Number timeout = luaL_optnumber(L, 2, -1);
	loski_ErrorCode err = loskiE_waitevent(drv, watcher, timeout);
	lua_settop(L, 1);  /* wch */
	if (!err) {
		size_t i = 0;
		loski_EventSource source;
		loski_EventValue event;
		lua_newtable(L);  /* wch,tab */
		while (loskiE_nextevent(drv, watcher, &i, &source, &event)) {
			const char *ename = EventName[event];
			pushsource(L, &source);  /* wch,tab,src */
			lua_pushvalue(L, -1);  /* wch,tab,src,src */
			lua_gettable(L, -3);  /* wch,tab,src,??? */
			if (lua_isnil(L, -1)) {  /* wch,tab,src,nil */
				lua_pop(L, 1);  /* wch,tab,src */
				lua_pushstring(L, ename);  /* wch,tab,src,evt */
				lua_settable(L, 2);  /* wch,tab */
			} else if (!strchr(lua_tostring(L, -1), *ename)) {  /* wch,tab,src,str */
				lua_pushstring(L, ename);  /* wch,tab,src,str,evt */
				lua_concat(L, 2);  /* wch,tab,src,str */
				lua_settable(L, 2);  /* wch,tab */
			} else {
				lua_settop(L,2);  /* wch,tab */
			}
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
	{"add", ew_add},
	{"remove", ew_remove},
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
