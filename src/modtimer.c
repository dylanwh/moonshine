#include "moonshine/config.h"
#include "moonshine/lua.h"

#include <glib.h>

#define CLASS "moonshine.timer"

struct luatimer {
    MSLuaRef *callback;
    guint tag, interval;
    /* This pointer is non-NULL if we are currently executing the timer proc.
     * If the pointed-to value is set to TRUE, the timer proc will avoid
     * dereferencing the timer structure again, and will return FALSE to
     * avoid re-running the timer.
     *
     * This is used to ensure we can safely GC the timer structure even
     * from within the timer proc itself. The alternative, using an indirect
     * pointer in LUA and flags here, causes additional heap fragmentation
     * and memory usage.
     */
    gboolean *destroyed_flag;
    /* gbooleans are 4 bytes, so save 4 bytes or so here with a bitfield */
    gboolean enabled : 1, 
             /* Set to TRUE if the active timer has been recreated in glib
              * (but not GC'd), and so we should return FALSE from the timer
              * proc.
              */
             quash_active_timer : 1;
};

static gboolean timer_cb(gpointer p_timer)/*{{{*/
{
    struct luatimer *timer = p_timer;
    gboolean destroyed_flag = FALSE;

    LuaState *L = ms_lua_pushref(timer->callback);

    g_assert(!timer->destroyed_flag);
    timer->quash_active_timer = FALSE;
    timer->destroyed_flag = &destroyed_flag;

    if (lua_pcall(L, 0, 1, 0)) {
        g_warning("moonshine error in timer callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        /* stop the timer from going off again, even if it's not a one-shot */
        lua_pushboolean(L, FALSE);
    }

    if (destroyed_flag)
        timer = NULL;
    else {
        g_assert(timer->destroyed_flag == &destroyed_flag);
        timer->destroyed_flag = NULL;
    }
    
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

    if (destroyed_flag || timer->quash_active_timer)
        return FALSE;

    timer->enabled = continue_timer;
    return timer->enabled;  
}

static void clear_timer(struct luatimer *timer)/*{{{*/
{
    if (timer->destroyed_flag) /* in timer proc */
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
    MSLuaRef *callback = ms_lua_ref_checktype(L, 2, LUA_TFUNCTION);

    struct luatimer *timer = ms_lua_newclass(L, CLASS, sizeof(*timer));

    timer->enabled = FALSE;
    timer->quash_active_timer = FALSE;
    timer->destroyed_flag = NULL;

    timer->callback = callback;
    timer->tag = timer->interval = 0;

    return 1;
}/*}}}*/

static int timer_schedule(LuaState *L)/*{{{*/
{
    struct luatimer *timer = ms_lua_checkclass(L, CLASS, 1);
    int interval = luaL_checkinteger(L, 2);
    
    if (interval <= 0) {
        return luaL_argerror(L, 2, "interval cannot be negative");
    }

    schedule_timer(timer, interval);

    return 0;
}/*}}}*/

static int timer_clear(LuaState *L)/*{{{*/
{
    struct luatimer *timer = ms_lua_checkclass(L, CLASS, 1);

    clear_timer(timer);

    return 0;
}/*}}}*/

static int timer_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    int pieces = 0;
    struct luatimer *timer = ms_lua_checkclass(L, CLASS, 1);

    sprintf(buff, "%p", timer);
    lua_pushfstring(L, "Timer (%s", buff);
    pieces++;

    if (timer->destroyed_flag) {
        lua_pushstring(L, ", in proc");
        pieces++;
    }
    if (timer->destroyed_flag && timer->quash_active_timer) {
        lua_pushstring(L, ", quashed");
        pieces++;
    }
    if (timer->enabled) {
        lua_pushfstring(L, ", enabled - interval %u tag %u",
                timer->interval, timer->tag);
        pieces++;
    }
    lua_pushstring(L, ")");
    pieces++;

    lua_concat(L, pieces);

    return 1;
}/*}}}*/

static int timer_gc(LuaState *L)/*{{{*/
{
    struct luatimer *timer = ms_lua_toclass(L, CLASS, 1);

    if (timer->enabled && !timer->destroyed_flag) {
        g_warning("Timer GC'd while still enabled");
    }
    clear_timer(timer);
    if (timer->destroyed_flag) {
        g_assert(timer->quash_active_timer);
        *timer->destroyed_flag = TRUE;
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
