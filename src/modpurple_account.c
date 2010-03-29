#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <account.h>

#define CLASS "purple.account"

/* {{{ Methods */
static int account_new(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *username = luaL_checkstring(L, 2);
    const char *proto_id = luaL_checkstring(L, 3);
    PurpleAccount **account = ms_lua_newclass(L, CLASS, sizeof(PurpleAccount *));
    *account  = purple_account_new(username, proto_id);
    ms_lua_backref_set(L, *account, -1);
    return 1;
}/*}}}*/

static int account_connect(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, CLASS, 1);
    purple_account_connect(*account);
    return 0;
}/*}}}*/

static int account_set_password(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, CLASS, 1);
    const char *password    = luaL_checkstring(L, 2);
    purple_account_set_password(*account, password);
    return 0;
}/*}}}*/

static int account_set_enabled(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, CLASS, 1);
    gboolean enabled        = ms_lua_checkboolean(L, 2);
    purple_account_set_enabled(*account, "moonshine", enabled);
    return 0;
}/*}}}*/

/** we fold the following methods into one set() method:
 * account_set_bool
 * account_set_int
 * account_set_string */
static int account_set(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_checkclass(L, CLASS, 1);
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
    sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
    lua_pushfstring(L, "Account (%s)", buff);
    return 1;
}/*}}}*/

static int account_gc(LuaState *L)/*{{{*/
{
    PurpleAccount **account = ms_lua_toclass(L, CLASS, 1);
    ms_lua_backref_unset(L, *account);
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg account_methods[] = {/*{{{*/
    { "new",                          account_new},
    { "connect",                      account_connect},
    { "set_password",                 account_set_password},
    { "set_enabled",                  account_set_enabled},
    { "set",                          account_set        },
#if 0
    { "add_buddy",                    account_add_buddy},
    { "get_active_status",            account_get_active_status},
    { "get_alias",                    account_get_alias},
    { "get_bool",                     account_get_bool},
    { "get_check_mail",               account_get_check_mail},
    { "get_connection",               account_get_connection},
    { "get_enabled",                  account_get_enabled},
    { "get_int",                      account_get_int},
    { "get_password",                 account_get_password},
    { "get_protocol_id",              account_get_protocol_id},
    { "get_protocol_name",            account_get_protocol_name},
    { "get_remember_password",        account_get_remember_password},
    { "get_status_types",             account_get_status_types},
    { "get_string",                   account_get_string},
    { "get_username",                 account_get_username},
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
    { "user_split_get_default_value", account_user_split_get_default_value},
    { "user_split_get_reverse",       account_user_split_get_reverse},
    { "user_split_get_separator",     account_user_split_get_separator},
    { "user_split_get_text",          account_user_split_get_text},
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
    ms_lua_class_register(L, CLASS, account_methods, account_meta);
    return 1;
}/*}}}*/
