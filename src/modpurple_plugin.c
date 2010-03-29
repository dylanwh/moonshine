#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <purple.h>

#define CLASS "purple.plugin"

/* {{{ Methods */
static int plugin_get_info(LuaState *L)/*{{{*/
{
    PurplePlugin **plugin  = ms_lua_checkclass(L, CLASS, 1);
    PurplePluginInfo *info = (*plugin)->info;

    lua_newtable(L);
    lua_pushstring(L, info->id);
    lua_setfield(L, -2, "id");
    lua_pushstring(L, info->name);
    lua_setfield(L, -2, "name");

    return 1;
}/*}}}*/

/* {{{ Meta Methods */
static int plugin_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
    lua_pushfstring(L, "plugin (%s)", buff);
    return 1;
}/*}}}*/

static int plugin_gc(UNUSED LuaState *L)/*{{{*/
{
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg plugin_methods[] = {/*{{{*/
    { "get_info",                     plugin_get_info},
    { 0, 0 }
};/*}}}*/

static const LuaLReg plugin_meta[] = {/*{{{*/
    {"__gc",       plugin_gc},
    {"__tostring", plugin_tostring},
    {0, 0}
};/*}}}*/

int luaopen_purple_plugin(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, CLASS, plugin_methods, plugin_meta);
    return 1;
}/*}}}*/
