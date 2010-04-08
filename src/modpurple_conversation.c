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
#include <moonshine/purple.h>
#include <purple.h>

/* {{{ Methods */
static int conversation_new(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);

    PurpleConversationType conv_type = luaL_checkinteger(L, 2);
    PurpleAccount **account          = ms_lua_checkclass(L, "purple.account", 3);
    const char *name                 = luaL_checkstring(L, 4);

    g_return_val_if_fail(conv_type == PURPLE_CONV_TYPE_IM || conv_type == PURPLE_CONV_TYPE_CHAT, 0);

    PurpleConversation **conv = ms_lua_newclass(L, "purple.conversation", sizeof(PurpleConversation *));
    *conv  = purple_conversation_new(conv_type, *account, name);
    ms_lua_backref_set(L, *conv, -1);

    return 1;
}/*}}}*/

static int conversation_get_name(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    lua_pushstring(L, purple_conversation_get_name(*conv));
    return 1;
}/*}}}*/

static int conversation_get_title(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    lua_pushstring(L, purple_conversation_get_title(*conv));
    return 1;
}/*}}}*/

static int conversation_get_nick(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    switch (purple_conversation_get_type(*conv)) {
        case PURPLE_CONV_TYPE_IM:
            lua_pushstring(L, purple_account_get_username(purple_conversation_get_account(*conv)));
            break;
        case PURPLE_CONV_TYPE_CHAT:
            lua_pushstring(L, purple_conv_chat_get_nick(PURPLE_CONV_CHAT(*conv)));
            break;
        default:
            g_assert_not_reached();
            break;
    }

    return 1;
}/*}}}*/

/* libpurple says:
 * Writes to a conversation window.
 *
 * This function should not be used to write
 * IM or chat messages. Use purple_conv_im_write() and purple_conv_chat_write()
 * instead. Those functions will most likely call this anyway, but they may do
 * their own formatting, sound playback, etc.
 *
 * This can be used to write generic messages, such as "so and so closed the conversation window."
 * */
static int conversation_write(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    const char *who           = luaL_checkstring(L,  2);
    const char *message       = luaL_checkstring(L,  3);
    PurpleMessageFlags flags  = luaL_checkinteger(L, 4);
    time_t mtime              = luaL_checkinteger(L, 5);
    g_return_val_if_fail(*conv, 0);

    purple_conversation_write(*conv, who, message, flags, mtime);

    return 0;
}/*}}}*/

static int conversation_send(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    char *message             = g_markup_escape_text(luaL_checkstring(L,  2), -1);
    g_return_val_if_fail(*conv, 0);

    switch (purple_conversation_get_type(*conv)) {
        case PURPLE_CONV_TYPE_IM:
            purple_conv_im_send(PURPLE_CONV_IM(*conv), message);
            break;
        case PURPLE_CONV_TYPE_CHAT:
            purple_conv_chat_send(PURPLE_CONV_CHAT(*conv), message);
            break;
        default:
            g_assert_not_reached();
    }
    g_free(message);
    return 0;
}/*}}}*/

static int conversation_get_type(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);

    switch (purple_conversation_get_type(*conv)) {
        case PURPLE_CONV_TYPE_IM:
            lua_pushstring(L, "im");
            break;
        case PURPLE_CONV_TYPE_CHAT:
            lua_pushstring(L, "chat");
            break;
        default:
            lua_pushnil(L);
            break;
    }

    return 1;
}/*}}}*/

static int conversation_get_account(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);

    PurpleAccount *account = purple_conversation_get_account(*conv);
    ms_lua_backref_push_or_newclass(L, account, "purple.account", sizeof(PurpleAccount *));
    return 1;
}/*}}}*/

static int conversation_get_topic(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    switch (purple_conversation_get_type(*conv)) {
        case PURPLE_CONV_TYPE_IM:
            lua_pushstring(L, "IM");
            break;
        case PURPLE_CONV_TYPE_CHAT:
            lua_pushstring(L, purple_conv_chat_get_topic(PURPLE_CONV_CHAT(*conv)));
            break;
        default:
            g_assert_not_reached();
            break;
    }
    return 1;
}/*}}}*/

static int conversation_destroy(LuaState *L)/*{{{*/
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    purple_conversation_destroy(*conv);
    /* reference is removed in conversations.conversation_destroy() */
    return 0;
}/*}}}*/

static int conversation_get_userlist(LuaState *L)
{
    PurpleConversation **conv = ms_lua_checkclass(L, "purple.conversation", 1);
    g_return_val_if_fail(*conv, 0);
    switch (purple_conversation_get_type(*conv)) {
        case PURPLE_CONV_TYPE_IM:
            lua_pushnil(L);
            break;
        case PURPLE_CONV_TYPE_CHAT: {
            PurpleConvChat *chat = PURPLE_CONV_CHAT(*conv);
            int i = 1;
            lua_newtable(L);
            for (GList *buddy = chat->in_room; buddy; buddy = g_list_next(buddy), i++) {
                ms_purple_push_cbuddy(L, (PurpleConvChatBuddy *)buddy->data);
                lua_rawseti(L, -2, i);
            }
            break;
        }
        default:
            g_assert_not_reached();
            break;
    }
    return 1;
}

/*}}}*/

/* {{{ Meta Methods */
static int conversation_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, "purple.conversation", 1));
    lua_pushfstring(L, "conversation (%s)", buff);
    return 1;
}/*}}}*/

static int conversation_gc(LuaState *L)
{
    PurpleConversation **conv = ms_lua_toclass(L, "purple.conversation", 1);
    ms_lua_backref_unset(L, *conv);
    return 0;
}

/* }}} */

static const LuaLReg conversation_methods[] = {/*{{{*/
    { "new",                          conversation_new         },
    { "destroy",                      conversation_destroy     },
    { "get_name",                     conversation_get_name    },
    { "get_nick",                     conversation_get_nick    },
    { "get_title",                    conversation_get_title   },
    { "get_type",                     conversation_get_type    },
    { "get_account",                  conversation_get_account },
    { "get_topic",                    conversation_get_topic   },
    { "write",                        conversation_write       },
    { "send",                         conversation_send        },
    { "get_userlist",                 conversation_get_userlist},
    { 0, 0 }
};/*}}}*/

static const LuaLReg conversation_meta[] = {/*{{{*/
    {"__tostring", conversation_tostring},
    {"__gc",       conversation_gc      },
    {0, 0}
};/*}}}*/

int luaopen_purple_conversation(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, "purple.conversation", conversation_methods, conversation_meta);
    return 1;
}/*}}}*/
