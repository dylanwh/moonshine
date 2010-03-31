/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

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

#include <glib.h>
#include "moonshine/lua.h"
#include "moonshine/lua_var.h"

int main()
{
    LuaState *L = ms_lua_newstate();
    lua_pushstring(L, "pants");
    lua_pushstring(L, "foobar");
    MSLuaVar *V = ms_lua_var_new(L, -1);
    lua_pop(L, 1);

    g_assert(strcmp(lua_tostring(L, -1), "pants") == 0);
    LuaState *L2 = ms_lua_var_push(V);
    g_assert(strcmp(lua_tostring(L, -1), "foobar") == 0);
    g_assert(L == L2);

    g_assert(V->count == 1);
    ms_lua_var_ref(V);
    g_assert(V->count == 2);
    ms_lua_var_unref(V);
    g_assert(V->count == 1);
    ms_lua_var_unref(V);
    lua_close(L);

    return 0;
}
