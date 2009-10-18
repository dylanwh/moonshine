#include "moonshine/config.h"
#include "moonshine/lua.h"

#include <glib.h>

typedef struct {
    MSLuaRef *callback;
    guint idle_tag;
    guint timer_tag;
} IdleCallContext;

static gboolean on_call(IdleCallContext *ctx, guint other_tag)/*{{{*/
{
    LuaState *L = ms_lua_pushref(ctx->callback);

    g_source_remove(other_tag);

    if (lua_pcall(L, 0, 0, 0)) {
        g_warning("moonshine error in idle callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    ms_lua_unref(ctx->callback);
    g_free(ctx);

    return FALSE;
}/*}}}*/

static gboolean on_idle(gpointer user_data)/*{{{*/
{
    IdleCallContext *ctx = user_data;
    return on_call(ctx, ctx->timer_tag);
}/*}}}*/

static gboolean on_timeout(gpointer user_data)/*{{{*/
{
    IdleCallContext *ctx = user_data;
    return on_call(ctx, ctx->idle_tag);
}/*}}}*/

/* moonshine.idle.call(func [, deadline])
 *
 * Call func when the main loop is idle (or when deadline ms elapse)
 */
static int idle_call(LuaState *L)/*{{{*/
{
    MSLuaRef *callback = ms_lua_ref_checktype(L, 1, LUA_TFUNCTION);
    int deadline       = CLAMP(luaL_optinteger(L, 2, 250), 0, 10000);

    if (deadline == 0) {
       /* theoretically, no deadline, but just set it to something big to
        * make the callback logic easier
        */
        deadline = 2000; /* 2 seconds */
    }
    IdleCallContext *ctx = g_new(IdleCallContext, 1);
    ctx->callback        = callback;
    ctx->idle_tag        = g_idle_add(on_idle, ctx);
    ctx->timer_tag       = g_timeout_add(deadline, on_timeout, ctx);

    return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
    {"call",  idle_call },
    { 0, 0 }
};/*}}}*/

int luaopen_moonshine_idle(LuaState *L)/*{{{*/
{
    ms_lua_module(L, functions);
    return 1;
}/*}}}*/
