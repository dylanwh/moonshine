#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/term.h>

#define CLASS "moonshine.ui.label"
typedef struct {
    char *text;
} Label;

static int label_new(LuaState *L)
{
    const char *text = luaL_optstring(L, 2, ""); 
    Label *t         = ms_lua_newclass(L, CLASS, sizeof(Label));
    t->text          = g_strdup(text);
    return 1;
}

static int label_set(LuaState *L)
{
    Label *t         = ms_lua_checkclass(L, CLASS, 1);
    const char *text = luaL_checkstring(L, 2);
    g_free(t->text);
    t->text = g_strdup(text);
    return 0;
}

static int label_render(LuaState *L)
{
    Label *t = ms_lua_checkclass(L, CLASS, 1);
    int row = luaL_checkinteger(L, 2);
    int cols = MS_TERM_COLS;

    ms_term_goto(row, 0);
    ms_term_style_set(0);
    ms_term_erase_eol();

    int width = 0;
    for (char *p = t->text; *p; p = g_utf8_next_char(p)) {
        gunichar ch = g_utf8_get_char(p);
        if (ch >= MS_TERM_STYLE_MIN_UCS && ch <= MS_TERM_STYLE_MAX_UCS) {
            ms_term_style_set(ch - MS_TERM_STYLE_MIN_UCS);
        } else {
            width += ms_term_charwidth(ch);
            if (width > cols)
                break;
            ms_term_write_gunichar(ch);
        }
    }
    for (; width < cols; width++)
        ms_term_write_gunichar(' ');

    ms_term_style_set(0);
    return 0;
}

static int label_tostring(LuaState *L)
{
    char buff[32];
    sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
    lua_pushfstring(L, "Label (%s)", buff);
    return 1;
}

static int label_gc(LuaState *L)
{
    Label *t = ms_lua_toclass(L, CLASS, 1);
    g_free(t->text);
    return 0;
}

static const LuaLReg label_methods[] = {
    {"new", label_new},
    {"render", label_render},
    {"set", label_set},
    {0, 0}
};

static const LuaLReg label_meta[] = {
    {"__gc", label_gc},
    {"__tostring", label_tostring},
    {0, 0}
};

int luaopen_moonshine_ui_label(LuaState *L)
{
    ms_lua_class_register(L, CLASS, label_methods, label_meta);
    return 1;
}
