#include "moon.h"
#include "term.h"
#include "config.h"
#include "util.h"

typedef struct {
	char *text;
} Statusbar;

static int Statusbar_new(LuaState *L)
{
	const char *text = luaL_optstring(L, 2, ""); 
	Statusbar *t         = moon_newclass(L, "Statusbar", sizeof(Statusbar));
	t->text          = g_strdup(text);
	return 1;
}

static int Statusbar_set(LuaState *L)
{
	Statusbar *t         = moon_checkclass(L, "Statusbar", 1);
	const char *text = luaL_checkstring(L, 2);
	g_free(t->text);
	t->text = g_strdup(text);
	return 0;
}

static int Statusbar_render(LuaState *L)
{
	Statusbar *t = moon_checkclass(L, "Statusbar", 1);
	int row = luaL_checkinteger(L, 2);
	int cols = TERM_COLS;

	term_goto(row, 0);
	term_color_use("default");
	term_erase_eol();

	int width = 0;
	for (char *p = t->text; *p; p = g_utf8_next_char(p)) {
		gunichar ch = g_utf8_get_char(p);
		if (ch >= BUFFER_COLOR_MIN_UCS && ch <= BUFFER_COLOR_MAX_UCS) {
			term_color_use_id(ch - BUFFER_COLOR_MIN_UCS);
		} else {
			width += unicode_charwidth(ch);
			if (width > cols)
				break;
			term_write_gunichar(ch);
		}
	}
	for (; width < cols; width++)
		term_write_gunichar(' ');

	term_color_use("default");
	return 0;
}

static int Statusbar_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Statusbar", 1));
  	lua_pushfstring(L, "Statusbar (%s)", buff);
  	return 1;
}

static int Statusbar_gc(LuaState *L)
{
	Statusbar *t = moon_toclass(L, "Statusbar", 1);
	g_free(t->text);
	return 0;
}

static const LuaLReg Statusbar_methods[] = {
	{"new", Statusbar_new},
	{"render", Statusbar_render},
	{"set", Statusbar_set},
	{0, 0}
};

static const LuaLReg Statusbar_meta[] = {
	{"__gc", Statusbar_gc},
	{"__tostring", Statusbar_tostring},
	{0, 0}
};

int luaopen_statusbar(LuaState *L)
{
	moon_class_register(L, "Statusbar", Statusbar_methods, Statusbar_meta);
	return 1;
}
