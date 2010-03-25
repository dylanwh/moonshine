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

static int term_init_pair(LuaState *L)
{
    guint16 id = luaL_checkint(L, 1);
    guint16 fg = luaL_checkint(L, 2);
    guint16 bg = luaL_checkint(L, 3);
    ms_term_init_pair(id, fg, bg);
    return 0;
}

static int term_init_color(LuaState *L)
{
    guint16 n = luaL_checkint(L, 1);
    guint16 r = luaL_checkint(L, 2);
    guint16 g = luaL_checkint(L, 3);
    guint16 b = luaL_checkint(L, 4);
    ms_term_init_color(n, r, g, b);
    return 0;
}

static int term_color_set(LuaState *L)
{
    guint16 id = luaL_checkint(L, 1);
    ms_term_color_set(id);
    return 0;
}

static int term_color_to_utf8(LuaState *L)
{
    guint16 id       = luaL_checkint(L, 1);
    const char *code = ms_term_color_to_utf8(id);
    lua_pushstring(L, code);
    return 1;
}

static int term_colors(LuaState *L)
{
    lua_pushinteger(L, COLORS);
    return 1;
}

static int term_color_pairs(LuaState *L)
{
    lua_pushinteger(L, COLOR_PAIRS);
    return 1;
}

static LuaLReg functions[] = {
    {"init",           term_init           },
    {"reset",          term_reset          },
    {"refresh",        term_refresh        },
    {"resize",         term_resize         },
    {"dimensions",     term_dimensions     },
    {"init_pair",      term_init_pair      },
    {"init_color",     term_init_color     },
    {"color_set",      term_color_set      },
    {"color_to_utf8",  term_color_to_utf8  },
    {"colors",         term_colors         },
    {"color_pairs",    term_color_pairs    },
    { 0, 0 },
};

int luaopen_moonshine_ui_term(LuaState *L)
{
    ms_lua_module(L, functions);
    lua_pushstring(L, MS_TERM_INDENT_MARK_UTF);
    lua_setfield(L, -2, "INDENT_MARK");

    return 1;
}

