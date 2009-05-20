#include "moonshine/config.h"
#include "moonshine/lua.h"

#include <glib.h>

#define CLASS "moonshine.timer"

struct luatimer {
	MSLuaRef *callback;
	guint tag, interval;
	/* gbooleans are 4 bytes, so save 12 bytes or so here with a bitfield */
	gboolean enabled : 1, 
			 /* If we need to cancel and recreate a timer from within the timer
			  * proc, we set quash_active_timer here. This will force the timer
			  * proc to return FALSE even if lua returns true, thus allowing
			  * us to cancel the timer without having to remove the source
			  * while it's in use
			  * 
			  * Note that once quash_active_timer is set, tag refers to a
			  * different, new source
			  *
			  * quash_active_timer is don't-care when in_timer_proc is 0
			  */
			 in_timer_proc : 1,
			 quash_active_timer : 1,
			 /* If the timer's lua side is destroyed with the timer proc active,
			  * this flag indicates the struct luatimer should be freed on
			  * return from the timer proc
			  */
			 free_on_return : 1;
};

static gboolean timer_cb(gpointer p_timer)/*{{{*/
{
	struct luatimer *timer = p_timer;
	LuaState *L = ms_lua_pushref(timer->callback);

	g_assert(!timer->in_timer_proc);
	timer->in_timer_proc = TRUE;
	timer->free_on_return = FALSE;
	timer->quash_active_timer = FALSE;

	if (lua_pcall(L, 0, 1, 0)) {
		g_warning("moonshine error in timer callback: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		/* stop the timer from going off again, even if it's not a one-shot */
		lua_pushboolean(L, FALSE);
	}
	g_assert(timer->in_timer_proc);
	timer->in_timer_proc = FALSE;
	
	if (!lua_isboolean(L, -1)) {
		if (lua_type(L, -1) != LUA_TNIL) {
			g_warning("bad return type from timer: %s",
					lua_typename(L, lua_type(L, -1)));
		}

		/* assume we want to stop the timer */
		lua_pop(L, 1);
		lua_pushboolean(L, FALSE);
	}

	gboolean continue_timer = lua_toboolean(L, -1);
	lua_pop(L, 1);
	
	if (timer->quash_active_timer) {
		if (timer->free_on_return) {
			if (continue_timer)
				g_warning("Attempted to continue a GC'd timer");

			g_free(timer);
		}
		return FALSE;
	}

	timer->enabled = continue_timer;
	return timer->enabled;	
}

static void clear_timer(struct luatimer *timer)/*{{{*/
{
	if (timer->in_timer_proc)
		timer->quash_active_timer = TRUE;
	else if (timer->enabled) {
		g_source_remove(timer->tag);
		timer->enabled = FALSE;
	} else {
		/* timer isn't enabled, nothing to do */
	}
}/*}}}*/

static void schedule_timer(struct luatimer *timer, gint interval)/*{{{*/
{
	clear_timer(timer);
	timer->interval = interval;
	timer->tag = g_timeout_add(interval, timer_cb, timer);
	timer->enabled = TRUE;
}/*}}}*/

static int timer_new(LuaState *L)/*{{{*/
{
	MSLuaRef *callback;

	luaL_checktype(L, 2, LUA_TFUNCTION);
	callback = ms_lua_ref(L, 2);

	struct luatimer **l_timer = ms_lua_newclass(L, CLASS, sizeof(*l_timer));
	struct luatimer *timer;
	timer = *l_timer = g_malloc(sizeof(*timer));

	timer->enabled = FALSE;
	timer->in_timer_proc = FALSE;
	timer->free_on_return = FALSE;
	timer->quash_active_timer = FALSE;

	timer->callback = callback;
	timer->tag = timer->interval = 0;

	return 1;
}/*}}}*/

static int timer_schedule(LuaState *L)/*{{{*/
{
	struct luatimer **l_timer = ms_lua_checkclass(L, CLASS, 1);
	int interval = luaL_checkinteger(L, 2);
	
	if (interval <= 0) {
		return luaL_argerror(L, 2, "interval cannot be negative");
	}

	schedule_timer(*l_timer, interval);

	return 0;
}/*}}}*/

static int timer_clear(LuaState *L)/*{{{*/
{
	struct luatimer **l_timer = ms_lua_checkclass(L, CLASS, 1);

	clear_timer(*l_timer);

	return 0;
}/*}}}*/

static int timer_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
	int pieces = 0;
	struct luatimer **l_timer = ms_lua_checkclass(L, CLASS, 1);

	sprintf(buff, "%p", *l_timer);
	lua_pushfstring(L, "Timer (%s", buff);
	pieces++;

	if ((*l_timer)->in_timer_proc) {
		lua_pushstring(L, ", in proc");
		pieces++;
	}
	if ((*l_timer)->in_timer_proc && (*l_timer)->quash_active_timer) {
		lua_pushstring(L, ", quashed");
		pieces++;
	}
	if ((*l_timer)->enabled) {
		lua_pushfstring(L, ", enabled - interval %u tag %u",
				(*l_timer)->interval, (*l_timer)->tag);
		pieces++;
	}
	lua_pushstring(L, ")");
	pieces++;

	lua_concat(L, pieces);

	return 1;
}/*}}}*/

static int timer_gc(LuaState *L)/*{{{*/
{
	struct luatimer **l_timer = ms_lua_toclass(L, CLASS, 1);
	struct luatimer *timer = *l_timer;

	if (timer->enabled && !timer->in_timer_proc) {
		g_warning("Timer GC'd while still enabled");
	}
	clear_timer(timer);
	if (timer->in_timer_proc) {
		g_assert(timer->quash_active_timer);
		timer->free_on_return = TRUE;
	} else {
		g_free(timer);
	}

	return 0;
}/*}}}*/

static const LuaLReg timer_methods[] = {/*{{{*/
	{ "new", timer_new },
	{ "schedule", timer_schedule },
	{ "clear", timer_clear },
	{ 0, 0 }
};/*}}}*/

static const LuaLReg timer_meta[] = {/*{{{*/
	{"__gc", timer_gc},
	{"__tostring", timer_tostring},
	{0, 0}
};/*}}}*/

int luaopen_moonshine_timer(LuaState *L)/*{{{*/
{
	ms_lua_class_register(L, CLASS, timer_methods, timer_meta);
	return 1;
}/*}}}*/
