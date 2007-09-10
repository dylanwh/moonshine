#include "moonshine.h"


static int format_parse(LuaState *L)
{
	const char *input = luaL_checkstring(L, 1);
	GString *out = g_string_sized_new(strlen(input));
	const gchar *p = input;

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
				g_string_append(out, BUFFER_INDENT_MARK_UTF);
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
						g_string_append(out, term_color_to_utf8(name));
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
}

static int format_escape(LuaState *L)
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
}


