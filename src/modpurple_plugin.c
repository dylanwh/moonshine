/* Moonshine - a Lua-based chat client
 *
 * Copyright (C) 2010 Dylan William Hardison
 *
 * This file is part of Moonshine.
 *
 * Moonshine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonshine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
 */

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
/*}}}*/

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
