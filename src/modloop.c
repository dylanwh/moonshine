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

#include "moonshine/config.h"
#include "moonshine/lua.h"
#include "moonshine/term.h"
#include "moonshine/signal.h"

#include <glib.h>

static int loop_run(LuaState *L)/*{{{*/
{
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
    g_return_val_if_fail(loop != NULL, 0);

    g_main_loop_run(loop);

    return 0;
}/*}}}*/

static int loop_quit(UNUSED LuaState *L)/*{{{*/
{
    GMainLoop *loop = ms_lua_stash_get(L, "loop");
    g_return_val_if_fail(loop != NULL, 0);

    g_main_loop_quit(loop);

    return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
    {"run",  loop_run  },
    {"quit", loop_quit },
    { 0, 0 },
};/*}}}*/

int luaopen_moonshine_loop(LuaState *L)/*{{{*/
{
    ms_lua_module(L, functions);
    return 1;
}/*}}}*/
