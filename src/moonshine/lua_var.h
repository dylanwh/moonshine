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

#ifndef __MOONSHINE_LUA_VAR_H__
#define __MOONSHINE_LUA_VAR_H__
#include "moonshine/lua.h"
#include "moonshine/config.h"

typedef struct MSLuaVar {
    LuaState *state;
    int slot;
    int count;
    gboolean use_env;
} MSLuaVar;

#define ms_lua_var_new(L, idx)            ms_lua_var_new_full(L, idx, LUA_TNONE, FALSE)
#define ms_lua_var_new_type(L, idx, type) ms_lua_var_new_full(L, idx, type, FALSE)

MSLuaVar *ms_lua_var_new_full(LuaState *L, int idx, int type, gboolean use_env);
LuaState *ms_lua_var_push(MSLuaVar *V);
void      ms_lua_var_free(MSLuaVar *V);
MSLuaVar *ms_lua_var_ref(MSLuaVar *V);
void      ms_lua_var_unref(MSLuaVar *V);

INLINE LuaState *ms_lua_var_get(MSLuaVar *V, const char *name)
{
    LuaState *L = ms_lua_var_push(V);
    lua_getfield(L, -1, name);
    lua_remove(L, -2);
    return L;
}

#endif
