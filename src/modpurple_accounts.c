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
#include <moonshine/signal.h>
#include <purple.h>

// static MSLuaVar *accounts_uiops_lua = NULL;

/* API: purple.conversations.init(uiops) */
static int accounts_init(UNUSED LuaState *L)
{
    //accounts_uiops_lua = ms_lua_var_new_type(L, 1, LUA_TTABLE);

    //purple_accounts_set_ui_ops(&accounts_uiops);
    purple_accounts_init();
    return 0;
}

static int accounts_add(LuaState *L)
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    g_return_val_if_fail(*account != NULL, 0);
    ms_lua_backref_set(L, *account, 1);

    /* now store a ref so we don't get gc'd */
    lua_pushlightuserdata(L, *account);
    lua_pushvalue(L, 1);
    lua_settable(L, LUA_REGISTRYINDEX);
    purple_accounts_add(*account);
    return 0;
}

static int accounts_remove(LuaState *L)
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    g_return_val_if_fail(*account != NULL, 0);
    lua_pushlightuserdata(L, *account);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    purple_accounts_remove(*account);
    return 0;
}

static int accounts_delete(LuaState *L)
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    g_return_val_if_fail(*account != NULL, 0);
    purple_accounts_delete(*account);
    *account = NULL;
    lua_pushlightuserdata(L, *account);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    return 0;
}

static int accounts_find(LuaState *L)
{
    const char *name     = luaL_checkstring(L, 1);
    const char *protocol = luaL_checkstring(L, 2);
    PurpleAccount *account = purple_accounts_find(name, protocol);
    if (account) {
        lua_pushlightuserdata(L, account);
        lua_gettable(L, LUA_REGISTRYINDEX);
    }
    else {
        lua_pushboolean(L, FALSE);
    }

    return 1;
}

static int accounts_get_all(LuaState *L)
{
    GList *accounts = purple_accounts_get_all();
    int i = 1;

    lua_newtable(L);
    while (accounts != NULL) {
        PurpleAccount *account = accounts->data;
        lua_pushlightuserdata(L, account);
        lua_gettable(L, LUA_REGISTRYINDEX);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            PurpleAccount **userdata = ms_lua_newclass(L, "purple.account", sizeof(PurpleAccount *));
            *userdata = account;
            lua_pushlightuserdata(L, account);
            lua_pushvalue(L, -2);
            lua_settable(L, LUA_REGISTRYINDEX);
        }
        lua_rawseti(L, -2, i++);
        accounts = g_list_next(accounts);
    }

    return 1;
}

static LuaLReg functions[] = {
    { "init",    accounts_init    },
    { "add",     accounts_add     },
    { "remove",  accounts_remove  },
    { "delete",  accounts_delete  },
    { "find",    accounts_find    },
    { "get_all", accounts_get_all },
    { 0, 0 },
};

int luaopen_purple_accounts(LuaState *L)
{
    ms_lua_module(L, functions);

    return 1;
}

