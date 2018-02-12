#include "luaosi/levtlib.h"


#include <assert.h>


#ifdef LOSI_DISABLE_EVTDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)	((losi_EventDriver *)lua_touserdata(L, \
                                       lua_upvalueindex(DRVUPV)))
#endif


/* watcher [, errmsg] = event.watcher() */
static int evt_watcher (lua_State *L)
{
	losi_EventDriver *drv = todrv(L);
	losi_EventWatcher *watcher = losi_newwatcher(L);
	losi_ErrorCode err = losiE_initwatcher(drv, watcher);
	if (!err) losi_enablewatcher(L);
	return losiL_doresults(L, 1, err);
}


#define tolwatcher(L)	((LuaWatcher *)luaL_checkudata(L, 1, LOSI_WATCHERCLS))

/* string = tostring(watcher) */
static int ew_tostring (lua_State *L)
{
	LuaWatcher *lw = tolwatcher(L);
	if (lw->refs == 0)
		lua_pushliteral(L, "event watcher (closed)");
	else
		lua_pushfstring(L, "event watcher (%p)", lw->watcher);
	return 1;
}


static void pushregtabs (lua_State *L, int idx)
{
	lua_getuservalue(L, idx);  /* key2src */
	lua_pushvalue(L, -1);
	lua_gettable(L, -2);       /* key2src[key2src] --> src2evt */
}

static int endwatcher (losi_EventDriver *drv, lua_State *L, LuaWatcher *lw)
{
	losi_ErrorCode err = losiE_endwatcher(drv, &lw->watcher);
	if (!err) {
		lw->refs = 0;  /* mark watcher as closed */
		/* free all registered sources */
		pushregtabs(L, 1);  /* key2src, src2evt */
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			lua_pop(L, 1);  /* pops 'evt' */
			losi_freeevtsrc(L, -1);
		}
		lua_pop(L, 2);  /* pops 'key2src' and 'src2evt' */
		/* discard registry tables */
		lua_pushnil(L);
		lua_setuservalue(L, 1);
	}
	return losiL_doresults(L, 0, err);
}

/* getmetatable(watcher).__gc(watcher) */
static int ew_gc(lua_State *L)
{
	losi_EventDriver *drv = todrv(L);
	LuaWatcher *lw = tolwatcher(L);
	if (lw->refs != 0) endwatcher(drv, L, lw);
	return 0;
}


static losi_EventWatcher *towatcher (lua_State *L) {
	LuaWatcher *lw = tolwatcher(L);
	if (lw->refs == 0) luaL_error(L, "attempt to use a closed event watcher");
	return &lw->watcher;
}

/* succ [, errmsg] = watcher:close() */
static int ew_close(lua_State *L)
{
	losi_EventDriver *drv = todrv(L);
	LuaWatcher *lw = tolwatcher(L);
	towatcher(L);  /* make sure argument is a valid watcher */
	if (lw->refs > 1) return losiL_doresults(L, 0, LOSI_ERRINUSE);
	return endwatcher(drv, L, lw);
}


#define pushsrckey(L,D,S)	(lua_pushinteger(L, losiE_getsourceid(D,S)))

static void pushevtname (lua_State *L, losi_EventFlags evtflags)
{
	switch (evtflags) {
		case LOSI_EVTFLAGS_INPUT:
			lua_pushliteral(L, "r");
			break;
		case LOSI_EVTFLAGS_OUTPUT:
			lua_pushliteral(L, "w");
			break;
		case LOSI_EVTFLAGS_INPUT|LOSI_EVTFLAGS_OUTPUT:
			lua_pushliteral(L, "rw");
			break;
		default:
			lua_pushnil(L);
			break;
	}
}


/* succ [, errmsg] = watcher:set(source [, events]) */
static int ew_set (lua_State *L) {
	losi_EventDriver *drv = todrv(L);
	losi_EventWatcher *watcher = towatcher(L);
	losi_EventSource src;
	losi_EventFlags evtflags = LOSI_EVTFLAGS_NONE;
	const char *events = luaL_optstring(L, 3, "");
	losi_ErrorCode err;
	int unreg, remove;
	for (; *events; ++events) switch (*events) {
		case 'r': evtflags |= LOSI_EVTFLAGS_INPUT; break;
		case 'w': evtflags |= LOSI_EVTFLAGS_OUTPUT; break;
		default: return luaL_error(L, "unknown event (got '%c')", *events);
	}
	lua_settop(L, 2);                              /* [1]=watcher [2]=src */
	pushregtabs(L, 1);                             /* [3]=key2src [4]=src2evt */
	lua_pushvalue(L, 2);                           /* [5]=src */
	lua_gettable(L, 4);                            /* [5]=src2evt[src] --> evt */
	unreg = lua_isnil(L, -1);
	remove = (evtflags == LOSI_EVTFLAGS_NONE);
	if (unreg && remove) return losiL_doresults(L, 0, LOSI_ERRNONE);
	err = losi_getevtsrc(L, 2, unreg, &src, evtflags);
	if (err) return losiL_doresults(L, 0, err);
	err = losiE_setwatch(drv, watcher, &src, evtflags);
	if (!err) {
		pushsrckey(L, drv, &src);                    /* [6]=key */
		unreg = remove;
		if (unreg) lua_pushnil(L);                   /* [7]=nil */
		else       lua_pushvalue(L, 2);              /* [7]=src */
		lua_settable(L, 3);                          /* key2src[key] = nil|src */
		lua_pushvalue(L, 2);                         /* [6]=src */
		if (unreg) lua_pushnil(L);                   /* [7]=nil */
		else       pushevtname(L, evtflags);         /* [7]=evt */
		lua_settable(L, 4);                          /* src2evt[src] = nil|evt */
	}
	if (unreg) losi_freeevtsrc(L, 2);
	return losiL_doresults(L, 0, err);
}


/* events = watcher:get(source) */
static int ew_get (lua_State *L) {
	towatcher(L);
	lua_settop(L, 2);                              /* [1]=watcher [2]=src */
	pushregtabs(L, 1);                             /* [3]=key2src [4]=src2evt */
	lua_pushvalue(L, 2);                           /* [5]=src */
	lua_gettable(L, 3);                            /* [5]=src2evt[src] --> evt */
	return 1;
}


/* [source, events] = next(watcher, [source]) */
static int ew_next (lua_State *L) {
	towatcher(L);
	lua_settop(L, 2);                              /* [1]=watcher [2]=src */
	pushregtabs(L, 1);                             /* [3]=key2src [4]=src2evt */
	lua_pushvalue(L, 2);                           /* [5]=src */
	if (lua_next(L, 4))
		return 2;
	else {
		lua_pushnil(L);
		return 1;
	}
}


/* for source, events in pairs(watcher) do ... end */
static int ew_pairs (lua_State *L) {
	towatcher(L);
	lua_pushcfunction(L, ew_next);
	lua_pushvalue(L, 1);
	lua_pushnil(L);
	return 3;
}


/* map [, errmsg] = watcher:wait([timeout]) */
static int ew_wait (lua_State *L) {
	losi_EventDriver *drv = todrv(L);
	losi_EventWatcher *watcher = towatcher(L);
	lua_Number timeout = luaL_optnumber(L, 2, -1);
	losi_ErrorCode err = losiE_waitevent(drv, watcher, timeout);
	lua_settop(L, 1);
	if (!err) {
		losi_EventIterator i;
		losi_EventSource src;
		losi_EventFlags evtflags;
		lua_getuservalue(L, 1);        /* [2]=key2src */
		lua_newtable(L);               /* [3]=map */
		losiE_inititerator(drv, watcher, &i);
		while (losiE_nextevent(drv, watcher, &i, &src, &evtflags)) {
			pushsrckey(L, drv, &src);    /* [4]=key */
			lua_gettable(L, 2);          /* [4]=src */
			if (!lua_isnil(L, -1)) {     /* ignore sources not registered */
				pushevtname(L, evtflags);  /* [5]=evt */
				lua_settable(L, 3);        /* map[src]=evt */
			}
			else lua_pop(L, 1);
		}
		lua_pushnil(L);
		if (lua_next(L, 3)) lua_pop(L, 2);  /* has some event */
		else err = LOSI_ERRUNFULFILLED;  /* no registered source event */
	}
	return losiL_doresults(L, 1, err);
}


#ifndef LOSI_DISABLE_EVTDRV
static int lfreedrv (lua_State *L) {
	losi_EventDriver *drv = (losi_EventDriver *)lua_touserdata(L, 1);
	losiE_freedrv(drv);
	return 0;
}
#endif

static const luaL_Reg wch[] = {
	{"__tostring", ew_tostring},
	{"__gc", ew_gc},
	{"__pairs", ew_pairs},
	{"set", ew_set},
	{"get", ew_get},
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
#ifndef LOSI_DISABLE_EVTDRV
	/* create sentinel */
	losi_EventDriver *drv;
	losi_ErrorCode err;
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (losi_EventDriver *)luaL_newsentinel(L, sizeof(losi_EventDriver),
	                                               lfreedrv);
	/* initialize library */
	err = losiE_initdrv(drv);
	if (err) {
		luaL_cancelsentinel(L);
		losiL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, LOSI_WATCHERCLS, NULL, wch, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);

#ifdef LOSI_ENABLE_WATCHEREVENTS
	losi_defgetevtsrc(L, LOSI_WATCHERCLS, losiE_getwatchevtsrc);
	losi_deffreeevtsrc(L, LOSI_WATCHERCLS, losiE_freewatchevtsrc);
#endif
	return 1;
}
