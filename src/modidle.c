#include "moonshine/config.h"
#include "moonshine/lua.h"

#include <glib.h>

struct idle_thunk {/*{{{*/
	MSLuaRef *callback;
	guint idle_tag, timer_tag;
};/*}}}*/

static gboolean cb_impl(struct idle_thunk *thunk, guint cancel_tag)/*{{{*/
{
	LuaState *L = ms_lua_pushref(thunk->callback);
	g_source_remove(cancel_tag);

	if (lua_pcall(L, 0, 0, 0)) {
		g_warning("moonshine error in idle callback: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	ms_lua_unref(thunk->callback);
	g_free(thunk);

	return FALSE;
}/*}}}*/

static gboolean cb_idle(gpointer p_thunk)/*{{{*/
{
	struct idle_thunk *thunk = p_thunk;
	return cb_impl(thunk, thunk->timer_tag);
}/*}}}*/

static gboolean cb_timer(gpointer p_thunk)/*{{{*/
{
	struct idle_thunk *thunk = p_thunk;
	return cb_impl(thunk, thunk->idle_tag);
}/*}}}*/

/* moonshine.idle:call(func [, deadline])
 *
 * Call func when the main loop is idle (or when deadline ms elapse)
 */
static int idle_call(LuaState *L)/*{{{*/
{
	int deadline = 250; /* ms */
	int argc = lua_gettop(L);
	MSLuaRef *pFunc;
	struct idle_thunk *thunk;

	if (argc < 2 || argc > 3) {
		return luaL_error(L, "Wrong number of arguments to Idle:call (got %d)", argc);
	}

	deadline = luaL_optinteger(L, 3, 250);
	if (deadline < 0)
		return luaL_argerror(L, 3, "Deadline cannot be negative");
	if (deadline == 0) {
	   /* theoretically, no deadline, but just set it to something big to
		* make the callback logic easier
		*/
		deadline = 2000; /* 2 seconds */
	}
	if (deadline > 10000) {
		/* clamp the deadline to something reasonable */
		deadline = 10000;
	}

	luaL_checktype(L, 2, LUA_TFUNCTION);

	pFunc = ms_lua_ref(L, 2);
	thunk = g_malloc(sizeof *thunk);

	thunk->callback = pFunc;
	thunk->idle_tag = g_idle_add(cb_idle, thunk);
	thunk->timer_tag = g_timeout_add(deadline, cb_timer, thunk);

	return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
	{"call",  idle_call },
	{ 0, 0 }
};/*}}}*/

int luaopen_moonshine_idle(LuaState *L)/*{{{*/
{
	lua_newtable(L);
	luaL_register(L, NULL, functions);
	return 1;
}/*}}}*/
