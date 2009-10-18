#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/term.h>
#include <moonshine/signal.h>

static int term_format(LuaState *L)/*{{{*/
{
    const char *input = luaL_checkstring(L, 1);
    GString *out = g_string_sized_new(strlen(input));
    const gchar *p = input;
    luaL_checktype(L, 2, LUA_TTABLE);

    while (1) {
        const gchar *oldp = p;
        gchar *nextesc = strchr(p, '%');
        if (!nextesc) {
            g_string_append(out, p);
            // XXX: Maybe use lua_pushlstring?
            lua_pushstring(L, out->str);
            g_string_free(out, TRUE);
            return 1;
        }
        g_string_append_len(out, p, nextesc - p);
        switch (*(nextesc + 1)) {
            case '%':
                g_string_append_c(out, '%');
                p = nextesc + 2;
                break;
            case '|':
                g_string_append(out, MS_TERM_INDENT_MARK_UTF);
                p = nextesc + 2;
                break;
            case '1' ... '9':
                {
                    lua_rawgeti(L, 2, *(nextesc + 1) - '0');
                    const char *s = lua_tostring(L, -1);
                    if (s != NULL)
                        g_string_append(out, s);
                    p = nextesc + 2;
                    lua_pop(L, 1);
                    break;
                }
            case '{':
                {
                    gchar *start = nextesc + 2;
                    gchar *end = strchr(start, '}');
                    if (end) {
                        gchar name[end - start + 1];
                        memcpy(name, start, sizeof name - 1);
                        name[sizeof name - 1] = '\0';
                        g_string_append(out, ms_term_color_to_utf8(name));
                        p = end + 1;
                        break;
                    } else { goto unknown_esc; }
                }
            default:
unknown_esc:
                g_string_append_c(out, *nextesc);
                p = nextesc + 1;
                break;
        }
        g_assert(p > oldp);
    }
    g_assert_not_reached();

}/*}}}*/

static int term_format_escape(LuaState *L)/*{{{*/
{
    const char *input = luaL_checkstring(L, 1);
    GString *out = g_string_sized_new(strlen(input));

    const gchar *p = input;
    while (1) {
        gchar *nextesc = strchr(p, '%');
        if (!nextesc) {
            g_string_append(out, p);
            lua_pushstring(L, out->str);
            g_string_free(out, TRUE);
            return 1;
        }
        g_string_append_len(out, p, nextesc - p);
        g_string_append(out, "%%");
        p = nextesc + 1;
    }
    g_assert_not_reached();
}/*}}}*/

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

static int term_status(LuaState *L)/*{{{*/
{
    const char *msg = luaL_checkstring(L, 1);
    printf("\e]2;%s\a", msg);
    fflush(stdout);
    return 0;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
    {"format",        term_format },
    {"format_escape", term_format_escape },
    {"refresh",       term_refresh },
    {"resize",        term_resize  },
    {"dimensions",    term_dimensions },
    {"defcolor",      term_defcolor },
    {"status",        term_status },
    { 0, 0 },
};/*}}}*/

int luaopen_moonshine_ui_term(LuaState *L)/*{{{*/
{
    ms_lua_module(L, functions);
    return 1;
}/*}}}*/

