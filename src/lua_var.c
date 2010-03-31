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

#include "moonshine/config.h"
#include "moonshine/lua_var.h"
#include <glib.h>

inline static int MS_LUA_VAR_IDX(gboolean use_env)/*{{{*/
{
    return use_env ? LUA_ENVIRONINDEX : LUA_REGISTRYINDEX;
}/*}}}*/

MSLuaVar *
ms_lua_var_new_full(LuaState *state, int idx, int type, gboolean use_env)
{
    MSLuaVar *var = NULL;

    if (type != LUA_TNONE)
        luaL_checktype(state, idx, type);

    lua_pushvalue(state, idx);
    int slot     = luaL_ref(state, MS_LUA_VAR_IDX(use_env));
    var          = g_new0(MSLuaVar, 1);
    var->state   = state;
    var->slot    = slot;
    var->use_env = use_env;
    var->count   = 1;
    return var;
}

LuaState *
ms_lua_var_push(MSLuaVar *var)/*{{{*/
{
    g_assert(var->count > 0);
    lua_rawgeti(var->state, MS_LUA_VAR_IDX(var->use_env), var->slot);
    return var->state;
}/*}}}*/

void
ms_lua_var_free(MSLuaVar *var)/*{{{*/
{
    luaL_unref(var->state, MS_LUA_VAR_IDX(var->use_env), var->slot);
    g_free(var);
}/*}}}*/

MSLuaVar *
ms_lua_var_ref(MSLuaVar *var)
{
    g_return_val_if_fail(var->count > 0, NULL);
    var->count++;
    return var;
}

void
ms_lua_var_unref(MSLuaVar *var)
{
    g_return_if_fail(var->count > 0);
    var->count--;
    if (var->count == 0)
        ms_lua_var_free(var);
}
/* }}} */
