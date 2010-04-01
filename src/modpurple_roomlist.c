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

static MSLuaVar *roomlist_uiops_lua = NULL;

static void on_roomlist_create(PurpleRoomlist *roomlist)
{
    LuaState *L = ms_lua_var_get(roomlist_uiops_lua, "roomlist_create");
    gboolean is_new = ms_lua_backref_push_or_newclass(L, roomlist, "purple.roomlist", sizeof(PurpleRoomlist *));
    if (is_new) {
        purple_roomlist_ref(roomlist);
        purple_roomlist_ref(roomlist);
    }
    ms_lua_call(L, 1, 0, "purple.roomlist in uiops.create");
}

static void on_roomlist_add_room(PurpleRoomlist *roomlist, PurpleRoomlistRoom *room)
{
    LuaState *L = ms_lua_var_get(roomlist_uiops_lua, "roomlist_add_room");
    gboolean is_new = ms_lua_backref_push_or_newclass(L, roomlist, "purple.roomlist", sizeof(PurpleRoomlist *));
    if (is_new) purple_roomlist_ref(roomlist);
    ms_lua_backref_push_or_newclass(L, room, "purple.room", sizeof(PurpleRoomlistRoom *));
    ms_lua_call(L, 2, 0, "purple.roomlist in uiops.roomlist_add_room");
}

static void on_roomlist_in_progress(PurpleRoomlist *roomlist, gboolean flag)
{
    LuaState *L = ms_lua_var_get(roomlist_uiops_lua, "roomlist_in_progress");
    gboolean is_new = ms_lua_backref_push_or_newclass(L, roomlist, "purple.roomlist", sizeof(PurpleRoomlist *));
    if (is_new) purple_roomlist_ref(roomlist);
    lua_pushboolean(L, flag);
    ms_lua_call(L, 2, 0, "purple.roomlist in uiops.roomlist_on_progress");
}

static PurpleRoomlistUiOps roomlist_uiops = {
    NULL,
    on_roomlist_create,
    NULL,
    on_roomlist_add_room,
    on_roomlist_in_progress,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* {{{ Methods */

/* for consistency, this is a "class" method, not a function. */
static int roomlist_init(LuaState *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TTABLE);
    g_return_val_if_fail(roomlist_uiops_lua == NULL, 0);

    roomlist_uiops_lua = ms_lua_var_new_type(L, 2, LUA_TTABLE);
    purple_roomlist_set_ui_ops(&roomlist_uiops);
    return 0;
}

static int roomlist_new(LuaState *L)/*{{{*/
{
    luaL_checktype(L, 1, LUA_TTABLE);
    PurpleAccount  **account  = ms_lua_checkclass(L, "purple.account", 2);
    PurpleRoomlist **roomlist = ms_lua_newclass(L, "purple.roomlist", sizeof(PurpleRoomlist *));
    *roomlist                 = purple_roomlist_new(*account);
    ms_lua_backref_set(L, *roomlist, -1);
    return 1;
}/*}}}*/

static int roomlist_join(LuaState *L)
{
    PurpleRoomlist **roomlist = ms_lua_checkclass(L, "purple.roomlist", 1);
    PurpleRoomlistRoom **room = ms_lua_checkclass(L, "purple.room", 2);

    purple_roomlist_room_join(*roomlist, *room);
    return 0;
}
/* }}} */

/* {{{ Meta Methods */
static int roomlist_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, "purple.roomlist", 1));
    lua_pushfstring(L, "Roomlist (%s)", buff);
    return 1;
}/*}}}*/

static int roomlist_gc(LuaState *L)/*{{{*/
{
    PurpleRoomlist **roomlist = ms_lua_toclass(L, "purple.roomlist", 1);
    ms_lua_backref_unset(L, *roomlist);
    purple_roomlist_unref(*roomlist);
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg roomlist_methods[] = {/*{{{*/
    { "new",                          roomlist_new   },
    { "join",                         roomlist_join  },
    { "init",                         roomlist_init  },
    { 0, 0 }
};/*}}}*/

static const LuaLReg roomlist_meta[] = {/*{{{*/
    {"__gc",       roomlist_gc},
    {"__tostring", roomlist_tostring},
    {0, 0}
};/*}}}*/

int luaopen_purple_roomlist(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, "purple.roomlist", roomlist_methods, roomlist_meta);
    return 1;
}/*}}}*/
