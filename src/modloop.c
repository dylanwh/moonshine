#include "moonshine/config.h"
#include "moonshine/lua.h"
#include "moonshine/term.h"
#include "moonshine/signal.h"

#include <glib.h>

static int loop_run(LuaState *L)/*{{{*/
{
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
    g_return_val_if_fail(loop != NULL, 0);

    g_main_loop_run(loop);

    return 0;
}/*}}}*/

static int loop_quit(UNUSED LuaState *L)/*{{{*/
{
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
    g_return_val_if_fail(loop != NULL, 0);

    g_main_loop_quit(loop);

    return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
    {"run",  loop_run  },
    {"quit", loop_quit },
    { 0, 0 },
};/*}}}*/

int luaopen_moonshine_loop(LuaState *L)/*{{{*/
{
    lua_newtable(L);
    luaL_register(L, NULL, functions);
    return 1;
}/*}}}*/
