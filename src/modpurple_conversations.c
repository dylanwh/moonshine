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
#include <moonshine/signal.h>
#include <moonshine/purple.h>
#include <moonshine/config.h>

static MSLuaVar *conversations_uiops_lua = NULL;

static void on_create(PurpleConversation *conv)/*{{{*/
{
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "conversation_create");
    ms_lua_backref_push(L, conv);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        PurpleConversation **ptr = ms_lua_newclass(L, "purple.conversation", sizeof(PurpleConversation *));
        *ptr = conv;
        lua_pushlightuserdata(L, conv);
        lua_pushvalue(L, -2);
        lua_settable(L, LUA_REGISTRYINDEX);
    }

    ms_lua_call(L, 1, 0, "purple.conversations in uiops.conversation_create");
}/*}}}*/

static void on_destroy(PurpleConversation *conv)/*{{{*/
{
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "conversation_destroy");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    ms_lua_call(L, 1, 0, "purple.conversations in uiops.conversation_destroy");

    lua_pushlightuserdata(L, conv);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}/*}}}*/

static void on_write_chat(PurpleConversation *conv,
                           const char *name,
                           const char *message,
                           PurpleMessageFlags flags,
                           time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L  = ms_lua_var_get(conversations_uiops_lua, "conversation_write_chat");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    lua_pushstring(L, name);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 5, 0, "purple.conversations in uiops.conversation_write_chat");
}

static void on_write_im(PurpleConversation *conv,
                        const char *name,
                        const char *message,
                        PurpleMessageFlags flags,
                        time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L    = ms_lua_var_get(conversations_uiops_lua, "conversation_write_im");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    lua_pushstring(L, name);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 5, 0, "purple.conversations in uiops.conversation_write_im");
}

static void on_write(PurpleConversation *conv,
                     const char *name,
                     const char *alias,
                     const char *message,
                     PurpleMessageFlags flags,
                     time_t mtime)
{
    g_assert(conversations_uiops_lua);
    LuaState *L    = ms_lua_var_get(conversations_uiops_lua, "conversation_write");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    lua_pushstring(L, name);
    lua_pushstring(L, alias);
    lua_pushstring(L, message);
    lua_pushinteger(L, flags);
    lua_pushinteger(L, mtime);
    ms_lua_call(L, 6, 0, "purple.conversations in uiops.conversation_write");
}

static gboolean on_has_focus(PurpleConversation *conv)
{
    g_assert(conversations_uiops_lua);
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "conversation_has_focus");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    ms_lua_call(L, 1, 1, "purple.conversations in uiops.conversation_has_focus");
    gboolean rv = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return rv;
}

static void on_present(PurpleConversation *conv)
{
    g_assert(conversations_uiops_lua);
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "conversation_present");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));
    ms_lua_call(L, 1, 0, "purple.conversations in uiops.conversation_present");
}



static void on_chat_add_users(PurpleConversation *conv, GList *cbuddies, gboolean new_arrivals)
{
    g_assert(conversations_uiops_lua);
    LuaState *L = ms_lua_var_get(conversations_uiops_lua, "conversation_chat_add_users");
    lua_pushlightuserdata(L, conv);
    lua_gettable(L, LUA_REGISTRYINDEX);
    g_assert(!lua_isnil(L, -1));

    lua_newtable(L);
    int i = 1;
    for (GList *buddy = cbuddies; buddy; buddy = g_list_next(buddy), i++) {
        ms_purple_push_cbuddy(L, (PurpleConvChatBuddy *)buddy->data);
        lua_rawseti(L, -2, i);
    }

    lua_pushboolean(L, new_arrivals);
    ms_lua_call(L, 3, 0, "purple.conversations in uiops.conversation_chat_add_users");
}


static PurpleConversationUiOps conversations_uiops = {
    on_create,
    on_destroy,
    on_write_chat,
    on_write_im,
    on_write,
    on_chat_add_users,
    NULL,                      /* chat_rename_user     */
    NULL,                      /* chat_remove_users    */
    NULL,                      /* chat_update_user     */
    on_present,
    on_has_focus,
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
    purple_conversations_init();

    return 0;
}

static LuaLReg functions[] = {
    { "init", conversations_init },
    { 0, 0 },
};

int luaopen_purple_conversations(LuaState *L)
{
    ms_lua_module(L, functions);

    return 1;
}

