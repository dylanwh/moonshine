#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <purple.h>

#define CLASS "purple.conversation"

/* {{{ Methods */
static int conversation_new(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);

    PurpleConversationType conv_type = luaL_checkinteger(L, 2);
    PurpleAccount **account          = ms_lua_checkclass(L, "purple.account", 3);
    const char *name                 = luaL_checkstring(L, 4);

    PurpleConversation **conv = ms_lua_newclass(L, CLASS, sizeof(PurpleConversation *));
    *conv  = purple_conversation_new(conv_type, *account, name);
    ms_lua_backref_set(L, *conv, -1);
    return 1;
}/*}}}*/

static int conversation_get_name(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, CLASS, 1);
    lua_pushstring(L, purple_conversation_get_name(*conv));
    return 1;
}/*}}}*/

/* {{{ Meta Methods */
static int conversation_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
    lua_pushfstring(L, "conversation (%s)", buff);
    return 1;
}/*}}}*/

static int conversation_gc(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_toclass(L, CLASS, 1);
    ms_lua_backref_unset(L, *conv);
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg conversation_methods[] = {/*{{{*/
    { "new",                          conversation_new},
    { "get_name",                     conversation_get_name},
    { 0, 0 }
};/*}}}*/

static const LuaLReg conversation_meta[] = {/*{{{*/
    {"__gc",       conversation_gc},
    {"__tostring", conversation_tostring},
    {0, 0}
};/*}}}*/

int luaopen_purple_conversation(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, CLASS, conversation_methods, conversation_meta);
    ms_lua_require(L, "purple.account");
    return 1;
}/*}}}*/
