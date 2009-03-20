#include <moonshine/config.h>
#include <moonshine/lua.h>
#include <moonshine/term.h>

#define CLASS "moonshine.ui.statusbar"
typedef struct {
	char *text;
} Statusbar;

static int statusbar_new(LuaState *L)
{
	const char *text = luaL_optstring(L, 2, ""); 
	Statusbar *t         = ms_lua_newclass(L, CLASS, sizeof(Statusbar));
	t->text          = g_strdup(text);
	return 1;
}

static int statusbar_set(LuaState *L)
{
	Statusbar *t         = ms_lua_checkclass(L, CLASS, 1);
	const char *text = luaL_checkstring(L, 2);
	g_free(t->text);
	t->text = g_strdup(text);
	return 0;
}

static int statusbar_render(LuaState *L)
{
	Statusbar *t = ms_lua_checkclass(L, CLASS, 1);
	int row = luaL_checkinteger(L, 2);
	int cols = MS_TERM_COLS;

	ms_term_goto(row, 0);
	ms_term_color_use("default");
	ms_term_erase_eol();

	int width = 0;
	for (char *p = t->text; *p; p = g_utf8_next_char(p)) {
		gunichar ch = g_utf8_get_char(p);
		if (ch >= MS_TERM_COLOR_MIN_UCS && ch <= MS_TERM_COLOR_MAX_UCS) {
			ms_term_color_use_id(ch - MS_TERM_COLOR_MIN_UCS);
		} else {
			width += ms_term_charwidth(ch);
			if (width > cols)
				break;
			ms_term_write_gunichar(ch);
		}
	}
	for (; width < cols; width++)
		ms_term_write_gunichar(' ');

	ms_term_color_use("default");
	return 0;
}

static int statusbar_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
  	lua_pushfstring(L, "Statusbar (%s)", buff);
  	return 1;
}

static int statusbar_gc(LuaState *L)
{
	Statusbar *t = ms_lua_toclass(L, CLASS, 1);
	g_free(t->text);
	return 0;
}

static const LuaLReg statusbar_methods[] = {
	{"new", statusbar_new},
	{"render", statusbar_render},
	{"set", statusbar_set},
	{0, 0}
};

static const LuaLReg statusbar_meta[] = {
	{"__gc", statusbar_gc},
	{"__tostring", statusbar_tostring},
	{0, 0}
};

int luaopen_moonshine_ui_statusbar(LuaState *L)
{
	ms_lua_class_register(L, CLASS, statusbar_methods, statusbar_meta);
	return 1;
}
