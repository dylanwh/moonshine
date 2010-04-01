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
    ms_lua_module(L, functions);

    return 1;
}

