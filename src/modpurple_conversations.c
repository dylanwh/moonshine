#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <moonshine/signal.h>
#include <purple.h>

static MSLuaVar *conversations_uiops_lua = NULL;

static void on_create_conversation(PurpleConversation *conv)
{
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "create_conversation");
    g_assert(lua_type(L, -1) == LUA_TFUNCTION);
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    g_assert(lua_type(L, -2) == LUA_TFUNCTION);
    g_assert(lua_type(L, -1) == LUA_TUSERDATA);
    ms_lua_call(L, 1, 0, "purple.conversations in uiops.create_conversation");
}

static void on_destroy_conversation(PurpleConversation *conv)
{
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "destroy_conversation");
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    ms_lua_call(L, 1, 0, "purple.conversations in uiops.destroy_conversation");
    ms_lua_backref_unset(L, conv);
}

static void on_write_chat(PurpleConversation *conv,
                          const char *name,
                          const char *message,
                          PurpleMessageFlags flags,
                          time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "write_chat");
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    lua_pushstring(L, name);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 5, 0, "purple.conversations in uiops.write_chat");
    lua_gc(L, LUA_GCCOLLECT, 0);
}

static void on_write_im(PurpleConversation *conv,
                          const char *name,
                          const char *message,
                          PurpleMessageFlags flags,
                          time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "write_im");
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    lua_pushstring(L, name);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 5, 0, "purple.conversations in uiops.write_im");
    lua_gc(L, LUA_GCCOLLECT, 0);
}

static gboolean on_has_focus(PurpleConversation *conv)
{
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "has_focus");
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    ms_lua_call(L, 1, 1, "purple.conversations in uiops.has_focus");
    gboolean rv = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return rv;
}

static void on_present(PurpleConversation *conv)
{
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "present");
    ms_lua_backref_push_or_newclass(L, conv, "purple.conversation", sizeof(PurpleConversation *));
    ms_lua_call(L, 1, 0, "purple.conversations in uiops.present");
}

static PurpleConversationUiOps conversations_uiops = {
    on_create_conversation,    /* create_conversation  */
    on_destroy_conversation,   /* destroy_conversation */
    on_write_chat,             /* write_chat           */
    on_write_im,               /* write_im             */
    NULL,                      /* write_conv           */
    NULL,                      /* chat_add_users       */
    NULL,                      /* chat_rename_user     */
    NULL,                      /* chat_remove_users    */
    NULL,                      /* chat_update_user     */
    on_present,                /* present              */
    on_has_focus,              /* has_focus            */
    NULL,                      /* custom_smiley_add    */
    NULL,                      /* custom_smiley_write  */
    NULL,                      /* custom_smiley_close  */
    NULL,                      /* send_confirm         */
    NULL,
    NULL,
    NULL,
    NULL
};

/* API: purple.conversations.init(uiops) */
static int conversations_init(LuaState *L)
{
    conversations_uiops_lua = ms_lua_var_new_type(L, 1, LUA_TTABLE);

    purple_conversations_set_ui_ops(&conversations_uiops);

    return 0;
}

static LuaLReg functions[] = {
    { "init", conversations_init },
    { 0, 0 },
};

int luaopen_purple_conversations(LuaState *L)
{
    ms_lua_module(L, functions);
    ms_lua_require(L, "purple.conversation");

    return 1;
}

