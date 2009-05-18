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

static int term_refresh(LuaState *L)/*{{{*/
{
	ms_term_refresh();
	return 0;
}/*}}}*/

static int term_resize(LuaState *L)/*{{{*/
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

static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer R) /*{{{*/
{
	if (cond & G_IO_IN) {
		do {
			gunichar c = ms_term_getkey();
			char buf[8];
			memset(buf, 0, sizeof(buf));
			g_unichar_to_utf8(c, buf);
			LuaState *L = ms_lua_pushref( (MSLuaRef *) R );
			lua_pushstring(L, buf);
			if (lua_pcall(L, 1, 0, 0) != 0) {
				g_warning("error in moonshine.ui.term input function: %s", lua_tostring(L, -1));
				return TRUE;
			}
		} while (ms_term_input_pending(-1));
		//moon_call(L, "input_reset_hook", "");
		return TRUE;
	}
	return FALSE;
}/* }}} */

static void on_resize (int signal, gpointer R)/*{{{*/
{
	LuaState *L = ms_lua_pushref( (MSLuaRef *) R);
	ms_term_resize();
	if (lua_pcall(L, 0, 0, 0) != 0) {
		g_warning("error in resize function: %s", lua_tostring(L, -1));
	}
}/*}}}*/

static int term_setup(LuaState *L)/*{{{*/
{
	static gboolean did_setup = FALSE;
	g_assert(did_setup == FALSE);

	luaL_checktype(L, 1, LUA_TTABLE);
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));

	lua_getfield(L, 1, "input");
	MSLuaRef *input_ref  = ms_lua_ref(L, -1);

	lua_getfield(L, 1, "resize");
	MSLuaRef *resize_ref = ms_lua_ref(L, -1);

	lua_pop(L, 2);

	g_assert(input_ref->ref != LUA_REFNIL);
	g_assert(resize_ref->ref != LUA_REFNIL);

	g_io_add_watch_full(input, G_PRIORITY_DEFAULT, G_IO_IN, on_input,
			(gpointer) input_ref, (GDestroyNotify) ms_lua_unref);
	ms_signal_catch(SIGWINCH, on_resize, (gpointer) resize_ref,
			(GDestroyNotify) ms_lua_unref);

	did_setup = TRUE;
	return 0;
}/*}}}*/

static int term_make_keyspec(LuaState *L)/* {{{ */
{
	const char *str = luaL_checkstring(L, 1);
	gsize len       = lua_objlen(L, 1); 
	GString *buf    = g_string_sized_new(len);

	for (gsize i = 0; i < len; i++) {
		if (str[i] == '^')
			g_string_append_c(buf, str[++i] ^ 64);
		else
			g_string_append_c(buf, str[i]);
	}
	lua_pushlstring(L, buf->str, buf->len);
	g_string_free(buf, TRUE);
	return 1;
}/*}}}*/

static LuaLReg functions[] = {/*{{{*/
	{"make_keyspec",  term_make_keyspec },
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
	lua_newtable(L);
	luaL_register(L, NULL, functions);
	return 1;
}/*}}}*/

