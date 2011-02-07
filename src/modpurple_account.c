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

/* {{{ Methods */
static int account_new(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *username = luaL_checkstring(L, 2);
    const char *proto_id = luaL_checkstring(L, 3);
    PurpleAccount **account = ms_lua_newclass(L, "purple.account", sizeof(PurpleAccount *));
    *account  = purple_account_new(username, proto_id);

    lua_pushlightuserdata(L, *account);
    lua_pushvalue(L, -2);
    lua_settable(L, LUA_REGISTRYINDEX);
    purple_accounts_add(*account);
    return 1;
}/*}}}*/

static int account_init(UNUSED LuaState *L)/*{{{*/
{
    purple_accounts_init();
    purple_accounts_get_all();
    return 0;
}/*}}}*/

static int account_find(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *name     = luaL_checkstring(L, 2);
    const char *protocol = luaL_checkstring(L, 3);

    PurpleAccount *account = purple_accounts_find(name, protocol);
    if (account) {
        lua_pushlightuserdata(L, account);
        lua_gettable(L, LUA_REGISTRYINDEX);
    }
    else {
        lua_pushboolean(L, FALSE);
    }

    return 1;
}/*}}}*/

static int account_get_all(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);
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
}/*}}}*/

static int account_delete(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    lua_pushlightuserdata(L, *account);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    purple_accounts_delete(*account);
    *account = NULL;
    return 0;
}/*}}}*/

static int account_connect(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    purple_account_connect(*account);
    return 0;
}/*}}}*/

static int account_set_password(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    const char *password    = luaL_checkstring(L, 2);
    purple_account_set_password(*account, password);
    return 0;
}/*}}}*/

static int account_set_enabled(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    gboolean enabled        = ms_lua_checkboolean(L, 2);
    purple_account_set_enabled(*account, "moonshine", enabled);
    return 0;
}/*}}}*/

static int account_get_protocol_id(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    lua_pushstring(L, purple_account_get_protocol_id(*account));
    return 1;
}/*}}}*/

static int account_get_username(LuaState *L)/*{{{*/
{
    PurpleAccount **account   = ms_lua_checkclass(L, "purple.account", 1);
    lua_pushstring(L, purple_account_get_username(*account));
    return 1;
}/*}}}*/

static int account_get_alias(LuaState *L)/*{{{*/
{
    PurpleAccount **account   = ms_lua_checkclass(L, "purple.account", 1);
    lua_pushstring(L, purple_account_get_alias(*account));
    return 1;
}/*}}}*/

static int account_join_chat(LuaState *L)
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    const char *name         = luaL_checkstring(L, 2);

    g_return_val_if_fail(purple_account_is_connected(*account), 0);

    PurpleConnection *pc = purple_account_get_connection(*account);
    PurpleConversation *conv = purple_find_conversation_with_account(
            PURPLE_CONV_TYPE_CHAT,
            name,
            *account);

    if (conv) {
        purple_conversation_present(conv);
    }
    else {
        conv = purple_conversation_new(
                PURPLE_CONV_TYPE_CHAT,
                *account,
                name);
        purple_conv_chat_left(PURPLE_CONV_CHAT(conv));
    }

    GHashTable *hash;
    PurpleChat *chat = purple_blist_find_chat(*account, name);
    if (chat == NULL) {
        PurplePluginProtocolInfo *info = PURPLE_PLUGIN_PROTOCOL_INFO(
                purple_connection_get_prpl(pc));
        if (info->chat_info_defaults != NULL)
            hash = info->chat_info_defaults(pc, name);
    } else {
        hash = purple_chat_get_components(chat);
    }

    serv_join_chat(pc, hash);
    if (chat == NULL && hash != NULL)
        g_hash_table_destroy(hash);

    return 0;
}

/** we fold the following methods into one set() method:
 * account_set_bool
 * account_set_int
 * account_set_string */
static int account_set(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, "purple.account", 1);
    const char *key         = luaL_checkstring(L, 2);

    switch (lua_type(L, 3)) {
        case LUA_TNUMBER:
            purple_account_set_int(*account, key, lua_tonumber(L, 3));
            break;
        case LUA_TBOOLEAN:
            purple_account_set_bool(*account, key, lua_toboolean(L, 3));
        case LUA_TSTRING:
            purple_account_set_string(*account, key, lua_tostring(L, 3));
        default:
            luaL_error(L, "unsupported type for set()");
            break;
    }
    return 0;
}/*}}}*/

/* }}} */

/* {{{ Meta Methods */
static int account_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, "purple.account", 1));
    lua_pushfstring(L, "account (%s)", buff);
    return 1;
}/*}}}*/

static int account_gc(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_toclass(L, "purple.account", 1);
    *account = NULL;
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg account_methods[] = {/*{{{*/
    { "new",                          account_new             },
    { "init",                         account_init            },
    { "find",                         account_find            },
    { "get_all",                      account_get_all         },
    { "delete",                       account_delete          },
    { "connect",                      account_connect         },
    { "set_password",                 account_set_password    },
    { "set_enabled",                  account_set_enabled     },
    { "set",                          account_set             },
    { "get_username",                 account_get_username    },
    { "get_alias",                    account_get_alias       },
    { "get_protocol_id",              account_get_protocol_id },
    { "join_chat",                    account_join_chat       },
#if 0
    { "add_buddy",                    account_add_buddy},
    { "get_active_status",            account_get_active_status},
    { "get_check_mail",               account_get_check_mail},
    { "get_connection",               account_get_connection},
    { "get_enabled",                  account_get_enabled},
    { "get_password",                 account_get_password},
    { "get_protocol_name",            account_get_protocol_name},
    { "get_remember_password",        account_get_remember_password},
    { "get_status_types",             account_get_status_types},
    { "get_string",                   account_get_string},
    { "get_int",                      account_get_int},
    { "get_bool",                     account_get_bool},
    { "is_connected",                 account_is_connected},
    { "option_get_default_bool",      account_option_get_default_bool},
    { "option_get_default_int",       account_option_get_default_int},
    { "option_get_default_string",    account_option_get_default_string},
    { "option_get_setting",           account_option_get_setting},
    { "option_get_text",              account_option_get_text},
    { "option_get_type",              account_option_get_type},
    { "register",                     account_register},
    { "remove_buddy",                 account_remove_buddy},
    { "request_close",                account_request_close},
    { "set_alias",                    account_set_alias},
    { "set_check_mail",               account_set_check_mail},
    { "set_protocol_id",              account_set_protocol_id},
    { "set_remember_password",        account_set_remember_password},
    { "set_username",                 account_set_username},
#endif
    { 0, 0 }
};/*}}}*/

static const LuaLReg account_meta[] = {/*{{{*/
    {"__gc",       account_gc},
    {"__tostring", account_tostring},
    {0, 0}
};/*}}}*/

int luaopen_purple_account(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, "purple.account", account_methods, account_meta);
    return 1;
}/*}}}*/
