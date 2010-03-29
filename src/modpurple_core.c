#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <moonshine/signal.h>
#include <purple.h>

/* glib eventloop stuff {{{*/
#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef struct _PurpleGLibIOClosure {
    PurpleInputFunction function;
    guint result;
    gpointer data;
} PurpleGLibIOClosure;

static void purple_glib_io_destroy(gpointer data)
{
    g_free(data);
}

static gboolean purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
    PurpleGLibIOClosure *closure = data;
    PurpleInputCondition purple_cond = 0;

    if (condition & PURPLE_GLIB_READ_COND)
        purple_cond |= PURPLE_INPUT_READ;
    if (condition & PURPLE_GLIB_WRITE_COND)
        purple_cond |= PURPLE_INPUT_WRITE;

    closure->function(closure->data, g_io_channel_unix_get_fd(source),
              purple_cond);

    return TRUE;
}

static guint glib_input_add(gint fd, PurpleInputCondition condition, PurpleInputFunction function,
                               gpointer data)
{
    PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
    GIOChannel *channel;
    GIOCondition cond = 0;

    closure->function = function;
    closure->data = data;

    if (condition & PURPLE_INPUT_READ)
        cond |= PURPLE_GLIB_READ_COND;
    if (condition & PURPLE_INPUT_WRITE)
        cond |= PURPLE_GLIB_WRITE_COND;

    channel = g_io_channel_unix_new(fd);
    closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
                          purple_glib_io_invoke, closure, purple_glib_io_destroy);

    g_io_channel_unref(channel);
    return closure->result;
}

static PurpleEventLoopUiOps glib_eventloop_uiops = 
{
    g_timeout_add,
    g_source_remove,
    glib_input_add,
    g_source_remove,
    NULL,
#if GLIB_CHECK_VERSION(2,14,0)
    g_timeout_add_seconds,
#else
    NULL,
#endif

    /* padding */
    NULL,
    NULL,
    NULL
};
/*}}}*/

static MSLuaVar *on_ui_init_lua = NULL;
static void on_ui_init(void)
{
    g_assert(on_ui_init_lua != NULL);
    LuaState *L = ms_lua_var_push(on_ui_init_lua);
    lua_getfield(L, -1, "ui_init");
    ms_lua_call(L, 0, 0, "purple.core ui_init");
    lua_pop(L, 1);
    g_debug("on_ui_init was called.");
}

static PurpleCoreUiOps core_uiops = {
    NULL,
    NULL,
    on_ui_init,
    NULL,

    /* padding */
    NULL,
    NULL,
    NULL,
    NULL
};

static int core_init(LuaState *L)
{
    g_debug("purple.core.init() was called.");

    purple_debug_set_enabled(FALSE);
    purple_core_set_ui_ops(&core_uiops);
    purple_eventloop_set_ui_ops(&glib_eventloop_uiops);

    on_ui_init_lua = ms_lua_var_new_type(L, 1, LUA_TTABLE);

    gboolean purple_core_init_ok = purple_core_init("moonshine");
    g_assert(purple_core_init_ok);

    /* Create and load the buddylist. */
    purple_set_blist(purple_blist_new());
    purple_blist_load();

    /* Load the preferences. */
    purple_prefs_load();

    /* Load the pounces. */
    purple_pounces_load();


    return 0;
}

static LuaLReg functions[] = {
    { "init", core_init },
    { 0, 0 },
};

int luaopen_purple_core(LuaState *L)
{
    ms_lua_module(L, functions);

    return 1;
}

