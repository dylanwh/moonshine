#include "moonshine/config.h"
#include "moonshine/lua.h"
#include "moonshine/term.h"
#include "moonshine/signal.h"

#include <glib.h>

/* defined in main.c */
extern GMainLoop *ms_main_loop;

static int loop_run(UNUSED LuaState *L)/*{{{*/
{
	g_assert(ms_main_loop != NULL);
	g_main_loop_run(ms_main_loop);

	return 0;
}/*}}}*/

static int loop_quit(UNUSED LuaState *L)/*{{{*/
{
	g_assert(ms_main_loop != NULL);
	g_main_loop_quit(ms_main_loop);

	return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
	{"run",  loop_run  },
	{"quit", loop_quit },
	{ 0, 0 },
};/*}}}*/

int luaopen_moonshine_loop(LuaState *L)/*{{{*/
{
	g_assert(ms_main_loop != NULL);
	lua_newtable(L);
	luaL_register(L, NULL, functions);
	return 1;
}/*}}}*/

