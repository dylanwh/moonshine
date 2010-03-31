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

#include <moonshine/parseopt.h>
#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>

static MSParseOptStatus short_option(gpointer baton, const char opt, const char *argument, UNUSED const char *start)/*{{{*/
{
    char opt_str[2] = "-\0";
    MSLuaVar *V = baton;
    LuaState *L = ms_lua_var_push(V);

    opt_str[0] = opt;
    lua_pushstring(L, opt_str);
    lua_pushstring(L, argument);
    lua_call(L, 2, 1);
    return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptStatus long_option(gpointer baton, const char *opt, const char *argument, UNUSED const char *start)/*{{{*/
{
    MSLuaVar *V = baton;
    LuaState *L = ms_lua_var_push(V);
    lua_pushstring(L, opt);
    lua_pushstring(L, argument);
    lua_call(L, 2, 1);
    return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptStatus literal_option(gpointer baton, const char *literal, UNUSED const char *start)/*{{{*/
{
    MSLuaVar *V = baton;
    LuaState *L = ms_lua_var_push(V);

    lua_pushnil(L);
    lua_pushstring(L, literal);
    lua_call(L, 2, 1);
    return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptCallbacks parseopt_cb = {
    .shortopt = short_option,
    .longopt  = long_option,
    .literalopt = literal_option
};

static int parseopt_parse(LuaState *L)
{
    const char *argstr = luaL_checkstring(L, 1);
    MSLuaVar *V        = ms_lua_var_new_full(L, 2, LUA_TFUNCTION, TRUE);
    const char *result = ms_parseopt_parse(V, argstr, &parseopt_cb);
    ms_lua_var_free(V);
    lua_pushstring(L, result);
    return 1;
}

static LuaLReg functions[] = {
    {"parse",  parseopt_parse },
    { 0, 0 },
};

int luaopen_moonshine_parseopt_core(LuaState *L)/*{{{*/
{
    ms_lua_module(L, functions);

    lua_pushinteger(L, MS_PARSEOPT_STOP);
    lua_setfield(L, -2, "STOP");
    lua_pushinteger(L, MS_PARSEOPT_NOARG);
    lua_setfield(L, -2, "NOARG");
    lua_pushinteger(L, MS_PARSEOPT_EATARG);
    lua_setfield(L, -2, "EATARG");

    return 1;
}/*}}}*/
