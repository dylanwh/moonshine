#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/term.h>
#include <moonshine/signal.h>

static int term_refresh(UNUSED LuaState *L)/*{{{*/
{
    ms_term_refresh();
    return 0;
}/*}}}*/

static int term_resize(UNUSED LuaState *L)/*{{{*/
{
    ms_term_resize();
    return 0;
}/*}}}*/

static int term_dimensions(LuaState *L)/*{{{*/
{
    lua_pushinteger(L, MS_TERM_LINES);
    lua_pushinteger(L, MS_TERM_COLS);
    return 2;
}/*}}}*/

static int term_defcolor(LuaState *L)/*{{{*/
{
    const char *name = luaL_checkstring(L, 1);
    const char *fg = luaL_checkstring(L, 2);
    const char *bg = luaL_checkstring(L, 3);
    ms_term_color_set(name, fg, bg);
    return 0;
}/*}}}*/

static int term_getcolor(LuaState *L)/*{{{*/
{
    const char *name = luaL_checkstring(L, 1);
    lua_pushstring(L, ms_term_color_to_utf8(name));
    return 1;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
    {"refresh",       term_refresh },
    {"resize",        term_resize  },
    {"dimensions",    term_dimensions },
    {"defcolor",      term_defcolor },
    {"getcolor",      term_getcolor },
    { 0, 0 },
};/*}}}*/

int luaopen_moonshine_ui_term(LuaState *L)/*{{{*/
{
    ms_lua_module(L, functions);
    return 1;
}/*}}}*/

