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
static int room_new(LuaState *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *roomname = luaL_checkstring(L, 2);

    PurpleRoomlistRoom **room = ms_lua_newclass(L, "purple.room", sizeof(PurpleRoomlistRoom *));
    *room = purple_roomlist_room_new(PURPLE_ROOMLIST_ROOMTYPE_ROOM, roomname, NULL);

    return 1;
}

static int room_get_name(LuaState *L)/*{{{*/
{
    PurpleRoomlistRoom **room = ms_lua_checkclass(L, "purple.room", 1);
    lua_pushstring(L, purple_roomlist_room_get_name(*room));
    return 1;
}/*}}}*/
/* }}} */

/* {{{ Meta Methods */
static int room_tostring(LuaState *L)/*{{{*/
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, "purple.room", 1));
    lua_pushfstring(L, "Room (%s)", buff);
    return 1;
}/*}}}*/

static int room_gc(LuaState *L)/*{{{*/
{
    PurpleRoomlistRoom **room = ms_lua_toclass(L, "purple.room", 1);
    ms_lua_backref_unset(L, *room);
    return 0;
}/*}}}*/
/* }}} */

static const LuaLReg room_methods[] = {/*{{{*/
    { "new",                               room_new      },
    { "get_name",                          room_get_name },
    { 0, 0 }
};/*}}}*/

static const LuaLReg room_meta[] = {/*{{{*/
    {"__gc",       room_gc},
    {"__tostring", room_tostring},
    {0, 0}
};/*}}}*/

int luaopen_purple_room(LuaState *L)/*{{{*/
{
    ms_lua_class_register(L, "purple.room", room_methods, room_meta);
    return 1;
}/*}}}*/
