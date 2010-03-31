/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

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
#include <moonshine/term.h>
#include <moonshine/log.h>
#include <moonshine/signal.h>

static int term_refresh(UNUSED LuaState *L)
{
    ms_term_refresh();
    return 0;
}

static int term_init(LuaState *L)
{
    g_log_set_default_handler(ms_log_handler, ms_lua_stash_get(L, "log"));
    ms_term_init();
    return 0;
}

static int term_reset(LuaState *L)
{
    MSLog *log = ms_lua_stash_get(L, "log");
    ms_term_reset();
    ms_log_replay(log, g_log_default_handler, NULL);
    return 0;
}

static int term_resize(UNUSED LuaState *L)
{
    ms_term_resize();
    return 0;
}

static int term_dimensions(LuaState *L)
{
    lua_pushinteger(L, MS_TERM_LINES);
    lua_pushinteger(L, MS_TERM_COLS);
    return 2;
}

static int term_style_init(LuaState *L)
{
    guint16 id = luaL_checkint(L, 1);
    guint16 fg = luaL_checkint(L, 2);
    guint16 bg = luaL_checkint(L, 3);
    ms_term_style_init(id, fg, bg);
    return 0;
}

static int term_color_init(LuaState *L)
{
    guint16 n = luaL_checkint(L, 1);
    guint16 r = luaL_checkint(L, 2);
    guint16 g = luaL_checkint(L, 3);
    guint16 b = luaL_checkint(L, 4);
    ms_term_color_init(n, r, g, b);
    return 0;
}

static int term_style_set(LuaState *L)
{
    guint16 id = luaL_checkint(L, 1);
    ms_term_style_set(id);
    return 0;
}

static int term_style_code(LuaState *L)
{
    guint16 id       = luaL_checkint(L, 1);
    const char *code = ms_term_style_code(id);
    lua_pushstring(L, code);
    return 1;
}

static int term_colors(LuaState *L)
{
    if (MS_TERM_COLORS == 8)
        lua_pushinteger(L, 16);
    else
        lua_pushinteger(L, MS_TERM_COLORS);
    return 1;
}

static int term_styles(LuaState *L)
{
    lua_pushinteger(L, MS_TERM_STYLES);
    return 1;
}

static int term_current_style(LuaState *L)
{
    lua_pushinteger(L, ms_term_style);
    return 1;
}

static int term_tigetstr(LuaState *L)
{
    const char *capname = luaL_checkstring(L, 1);
    char *str = tigetstr(capname);
    if (str == (char *)-1)
        lua_pushnil(L);
    else
        lua_pushstring(L, str);

    return 1;
}

static LuaLReg functions[] = {
    {"init",           term_init           },
    {"reset",          term_reset          },
    {"refresh",        term_refresh        },
    {"resize",         term_resize         },
    {"dimensions",     term_dimensions     },
    {"style_init",      term_style_init      },
    {"color_init",     term_color_init     },
    {"style_set",      term_style_set      },
    {"style_code",     term_style_code  },
    {"colors",         term_colors         },
    {"styles",         term_styles    },
    {"current_style",  term_current_style },
    {"tigetstr",       term_tigetstr },
    { 0, 0 },
};

int luaopen_moonshine_ui_term(LuaState *L)
{
    ms_lua_module(L, functions);
    lua_pushstring(L, MS_TERM_INDENT_MARK_UTF);
    lua_setfield(L, -2, "INDENT_CODE");

    lua_pushstring(L, MS_TERM_STYLE_RESET_UTF);
    /*
    char buf[8];
    memset(buf, 0, sizeof(buf));
    g_unichar_to_utf8(MS_TERM_STYLE_RESET_UCS, buf);
    lua_pushstring(L, buf);
    */
    lua_setfield(L, -2, "STYLE_RESET_CODE");


    return 1;
}

