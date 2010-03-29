#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/lua_var.h>
#include <moonshine/signal.h>
#include <purple.h>

static MSLuaVar *conversations_uiops_lua = NULL;

static void on_write_conv(PurpleConversation *conv,
                          const char *name,
                          const char *alias,
                          const char *message,
                          PurpleMessageFlags flags,
                          time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "write_conv");
    PurpleConversation **ptr = ms_lua_newclass(L, "purple.conversation", sizeof(PurpleConversation *)); 
    *ptr = conv;
    lua_pushstring(L, name);
    lua_pushstring(L, alias);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 6, 0, "purple.conversations in uiops.write_conv");
}

static PurpleConversationUiOps conversations_uiops = {
    NULL,                      /* create_conversation  */
    NULL,                      /* destroy_conversation */
    NULL,                      /* write_chat           */
    NULL,                      /* write_im             */
    on_write_conv,             /* write_conv           */
    NULL,                      /* chat_add_users       */
    NULL,                      /* chat_rename_user     */
    NULL,                      /* chat_remove_users    */
    NULL,                      /* chat_update_user     */
    NULL,                      /* present              */
    NULL,                      /* has_focus            */
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

