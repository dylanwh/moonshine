#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <moonshine/config.h>
#include <purple.h>

/* API: list = purple.plugins.get_protocols() */
static int plugins_get_protocols(LuaState *L)
{
    lua_newtable(L); // protocol list
    GList *iter = purple_plugins_get_protocols();
    for (int i = 1; iter != NULL; iter = g_list_next(iter), i++) {
        PurplePlugin **plugin = ms_lua_newclass(L, "purple.plugin", sizeof(PurplePlugin *));
        *plugin = iter->data;
        lua_rawseti(L, -2, i);
    }

    return 1;
}

static LuaLReg functions[] = {
    { "get_protocols", plugins_get_protocols },
    { 0, 0 },
};

int luaopen_purple_plugins(LuaState *L)
{
    g_debug("luaopen_purple_plugins()");
    ms_lua_module(L, functions);
    ms_lua_require(L, "purple.plugin");

    return 1;
}

