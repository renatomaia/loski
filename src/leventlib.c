#include <stdio.h>

#include "eventlib.h"
#include "loskiaux.h"
#include "lnetaux.h"


#ifdef LOSKI_DISABLE_EVTDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)	((loski_EventDriver *)lua_touserdata(L, \
                                        lua_upvalueindex(DRVUPV)))
#endif


#define WATCHER_CLASS LOSKI_PREFIX"EventWatcher"
#define WATCHER_REGISTRY LOSKI_PREFIX"WatchedMapRegistry"


#define pushresults(L,n,e) luaL_pushresults(L,n,e,loski_eventerror)


static const char *const WatchableEvents[3][3] = {
	{"read", "write", NULL},  /* file events */
	{"read", "write", NULL},  /* socket events */
	{"exited", NULL, NULL},  /* process events */
};

static void checkWatch(lua_State *L,
                       loski_EventWatch *watch,
                       loski_WatchableReference *ref) {
	lua_settop(L, 3);
	if (loski_issocket(L, 2, LOSKI_SOCKTYPE_SOCK)) {
		watch->kind = LOSKI_WATCHSOCKET;
		ref->socket = loski_tosocket(L, 2, LOSKI_SOCKTYPE_SOCK);
	}
	else luaL_argerror(L, 2, "invalid watchable object");
	watch->event = luaL_checkoption(L, 3, NULL, WatchableEvents[watch->kind]);
}

static void pushWatchedMap(lua_State *L, int pidx) {
	lua_getfield(L, LUA_REGISTRYINDEX, WATCHER_REGISTRY);  /* ...,dat */
	luaL_pushobjtab(L, lua_gettop(L), pidx);               /* ...,dat,map */
	lua_remove(L, -2);                                     /* ...,map */
}

static void saveWatchObjRef(lua_State *L, loski_EventWatch *watch) {
	pushWatchedMap(L, 1);               /* pol,obj,evt,map */
	/* get reference registry */
	lua_pushinteger(L, watch->kind);    /* pol,obj,evt,map,knd */
	luaL_pushobjtab(L, 4, 5);           /* pol,obj,evt,map,knd,ref */
	/* save object reference */
	loski_pushwachedkey(L, watch);      /* pol,obj,evt,map,knd,ref,key */
	lua_pushvalue(L, 2);                /* pol,obj,evt,map,knd,ref,key,obj */
	lua_settable(L, -3);                /* pol,obj,evt,map,knd,ref */
	lua_pop(L, 2);                      /* pol,obj,evt,map */
	/* get event registry for this object */
	luaL_pushobjtab(L, 4, 2);           /* pol,obj,evt,map,reg */
	/* save object event */
	lua_pushboolean(L, 1);              /* pol,obj,evt,map,reg,tru */
	lua_rawseti(L, -2, watch->event);   /* pol,obj,evt,map,reg */
	lua_pop(L, 4);                      /* pol */
}

static int deleteWatchObjRef(lua_State *L, loski_EventWatch *watch) {
	int found = 0;                          /* pol,obj,evt */
	pushWatchedMap(L, 1);                   /* pol,obj,evt,map */
	/* get event registry for this object */
	lua_pushvalue(L, 2);                    /* pol,obj,evt,map,obj */
	lua_gettable(L, -2);                    /* pol,obj,evt,map,reg */
	if (lua_istable(L, -1)) {               /* pol,obj,evt,map,reg */
		/* find event in the registry */
		lua_rawgeti(L, -2, watch->event);     /* pol,obj,evt,map,reg,bol */
		found = lua_toboolean(L, -1);         /* pol,obj,evt,map,reg,bol */
		lua_pop(L, 1);                        /* pol,obj,evt,map,reg */
		if (found) {
			/* remove the event from the register */
			lua_pushnil(L);                     /* pol,obj,evt,map,reg,nil */
			lua_rawseti(L, -2, watch->event);   /* pol,obj,evt,map,reg */
			/* check if there is any event left in the registry */
			lua_pushnil(L);                     /* pol,obj,evt,map,reg,nil */
			if (lua_next(L, -2) == 0) {         /* pol,obj,evt,map,reg */
				/* no event left, discard registry for this object */
				lua_pushvalue(L, 2);              /* pol,obj,evt,map,reg,obj */
				lua_pushnil(L);                   /* pol,obj,evt,map,reg,obj,nil */
				lua_settable(L, -4);              /* pol,obj,evt,map,reg */
				/* get reference registry */
				lua_rawgeti(L, -2, watch->kind);  /* pol,obj,evt,map,reg,ref */
				if (lua_istable(L, -1)) {         /* pol,obj,evt,map,reg,ref */
					/* free object reference */
					loski_pushwachedkey(L, watch);  /* pol,obj,evt,map,reg,ref,key */
					lua_pushnil(L);                 /* pol,obj,evt,map,reg,ref,key,nil */
					lua_settable(L, -3);            /* pol,obj,evt,map,reg,ref */
					lua_pop(L, 1);                  /* pol,obj,evt,map,reg */
				}
			} else {                            /* pol,obj,evt,map,reg,key,val */
				lua_pop(L, 2);                    /* pol,obj,evt,map,reg */
			}
		}
	}
	lua_pop(L, 4);                          /* pol */
	return found;
}

static int pushWatchObjRef(lua_State *L, loski_EventWatch *watch) {
	pushWatchedMap(L, 1);             /* pol,...,map */
	/* get reference registry */
	lua_rawgeti(L, -1, watch->kind);  /* pol,...,map,ref */
	if (!lua_istable(L, -1)) {        /* pol,...,map,ref */
		lua_pop(L, 2);                  /* pol,... */
		return 0;
	}
	lua_remove(L, -2);                /* pol,...,ref */
	loski_pushwachedkey(L, watch);    /* pol,...,ref,key */
	lua_gettable(L, -2);              /* pol,...,ref,obj */
	lua_remove(L, -2);                /* pol,...,obj */
	return 1;
}


typedef struct LuaWatcher {
	loski_EventWatcher watcher;
	int closed;
} LuaWatcher;


#define tolwatcher(L)	((LuaWatcher *)luaL_checkinstance(L, 1, WATCHER_CLASS))


static int ew_tostring (lua_State *L)
{
	LuaWatcher *p = tolwatcher(L);
	if (p->closed)
		lua_pushliteral(L, "event watcher (closed)");
	else
		lua_pushfstring(L, "event watcher (%p)", p);
	return 1;
}


static loski_EventWatcher *towatcher (lua_State *L) {
	LuaWatcher *p = tolwatcher(L);
	if (p->closed)
		luaL_error(L, "attempt to use a closed event watcher");
	return &p->watcher;
}


static int aux_close(loski_EventDriver *drv, lua_State *L, LuaWatcher *p)
{
	int res = loski_endwatcher(drv, &p->watcher);
	if (res == 0) p->closed = 1;  /* mark watcher as closed */
	return pushresults(L, 0, res);
}


static int ew_close(lua_State *L)
{
	loski_EventDriver *drv = todrv(L);
	LuaWatcher *p = tolwatcher(L);
	towatcher(L);  /* make sure argument is a valid watcher */
	return aux_close(drv, L, p);
}


static int ew_gc(lua_State *L)
{
	loski_EventDriver *drv = todrv(L);
	LuaWatcher *p = tolwatcher(L);
	if (!p->closed) aux_close(drv, L, p);
	return 0;
}


static LuaWatcher *newwatcher (lua_State *L) {
	LuaWatcher *p = (LuaWatcher *)lua_newuserdata(L, sizeof(LuaWatcher));
	p->closed = 1;
	luaL_setmetatable(L, WATCHER_CLASS);
	return p;
}


/* watcher [, errmsg] = event.watcher() */
static int evt_watcher (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	LuaWatcher *lw = newwatcher(L);
	int err = loskiE_initwatcher(drv, &lw->watcher);
	if (err == 0) lw->closed = 0;
	return luaL_doresults(L, 1, err);
}


/* succ [, errmsg] = watcher:add(object [, event]) */
static int ew_add (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	loski_EventWatch watch;
	loski_WatchableReference ref;
	int res;
	checkWatch(L, &watch, &ref);
	res = loski_addwatch(drv, watcher, &watch, &ref);
	if (res == 0) saveWatchObjRef(L, &watch);
	return pushresults(L, 1, res);
}


/* succ [, errmsg] = watcher:remove(object [, event]) */
static int ew_remove (lua_State *L) {
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	loski_EventWatch watch;
	loski_WatchableReference ref;
	int res;
	checkWatch(L, &watch, &ref);
	res = loski_delwatch(drv, watcher, &watch, &ref);
	if (res == 0 && !deleteWatchObjRef(L, &watch)) {
		lua_pushnil(L);
		lua_pushliteral(L, "not found");
		return 2;
	}
	return pushresults(L, 1, res);
}


/* map [, errmsg] = watcher:wait([max [, timeout]]) */
static int ew_wait (lua_State *L) {
	size_t qsize;
	void *queue;
	int res;
	loski_EventDriver *drv = todrv(L);
	loski_EventWatcher *watcher = towatcher(L);
	lua_Number timeout = luaL_optnumber(L, 2, -1);
	size_t count = luaL_optint(L, 3, 0);
	lua_settop(L, 3);                                  /* wat,max,tim */
	qsize = loski_eventqueuesize(drv, watcher, count);
	queue = lua_newuserdata(L, qsize);                 /* wat,max,tim,usd */
	res = loski_waitevent(drv, watcher, queue, &count, timeout);
	if (res == 0) {
		size_t i;
		lua_newtable(L);                                 /* wat,max,tim,usd,tab */
		for (i = 0; i < count; ++i) {
			loski_EventWatch watch;
			res = loski_getevent(drv, &watch, queue, i);
			if (res == 0 && pushWatchObjRef(L, &watch)) {  /* wat,max,tim,usd,tab,obj */
				const char *ename = WatchableEvents[watch.kind][watch.event];
				luaL_pushobjtab(L, 5, 6);                    /* wat,max,tim,usd,tab,obj,set */
				lua_pushstring(L, ename);                    /* wat,max,tim,usd,tab,obj,set,str */
				lua_pushboolean(L, 1);                       /* wat,max,tim,usd,tab,obj,set,str,tru */
				lua_settable(L, 7);                          /* wat,max,tim,usd,tab,obj,set */
				lua_pop(L, 2);                               /* wat,max,tim,usd,tab */
			}
		}
		return 1;                                        /* wat,max,tim,usd,tab */
	}
	return pushresults(L, 2, res);
}

#ifndef LOSKI_DISABLE_EVTDRV
static int lfreedrv (lua_State *L) {
	loski_EventDriver *drv = (loski_EventDriver *)lua_touserdata(L, 1);
	loski_closeevents(drv);
	return 0;
}
#endif

static luaL_Reg ew[] = {
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
	int err;
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (loski_EventDriver *)luaL_newsentinel(L, sizeof(loski_EventDriver),
	                                               lfreedrv);
	/* initialize library */
	err = loskiE_initdrv(drv);
	if (err != 0) {
		luaL_cancelsentinel(L);
		luaL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create abstract base socket class */
	pushsentinel(L);
	luaL_newclass(L, WATCHER_CLASS, NULL, ew, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);
	/* create poll member registry */
	lua_newtable(L);
	lua_createtable(L, 0, 1);
	lua_pushliteral(L, "k");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, WATCHER_REGISTRY);
	return 1;
}
