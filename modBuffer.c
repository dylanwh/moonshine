#include "moonshine.h"
#include "buffer.h"

#include <string.h>

#define BUFFER "Buffer"

static Buffer *toBuffer (lua_State *L, int index)
{
  	Buffer **e = lua_touserdata(L, index);
  	if (e == NULL) luaL_typerror(L, index, BUFFER);
  	return *e;
}

static Buffer *checkBuffer (lua_State *L, int index)
{
  	Buffer **e;
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	e = (Buffer **)luaL_checkudata(L, index, BUFFER);
  	if (e == NULL) luaL_typerror(L, index, BUFFER);
  	return *e;
}

static Buffer *pushBuffer (lua_State *L, guint size)
{
  	Buffer **e = (Buffer **)lua_newuserdata(L, sizeof(Buffer *));
  	luaL_getmetatable(L, BUFFER);
  	lua_setmetatable(L, -2);
  	*e = buffer_new(size);
  	return *e;
}

static int Buffer_new (lua_State *L)
{
	guint size = luaL_optint(L, 1, 1024);
  	pushBuffer(L, size);
  	return 1;
}

static int Buffer_set_histsize(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	guint size = luaL_checkinteger(L, 2);
	buffer_set_history_max(b, size);
	return 0;
}

static int Buffer_get_histsize(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	lua_pushinteger(L, buffer_get_history_max(b));
	return 1;
}

static int Buffer_render(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	buffer_render(b);
	return 0;
}

static int Buffer_print(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	const char *s = luaL_checkstring(L, 2);
	buffer_print(b, s);
	return 0;
}

static int Buffer_scroll(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	int i = luaL_checkinteger(L, 2);
	buffer_scroll(b, i);
	return 0;
}

static int Buffer_scroll_to(LuaState *L)
{
	Buffer *b = checkBuffer(L, 1);
	int i = luaL_checkinteger(L, 2);
	buffer_scroll_to(b, i);
	return 0;
}

static int Buffer_format(LuaState *L) {
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
						g_string_append(out, name); //term_color_to_utf8(name));
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

static int Buffer_format_escape(LuaState *L) {
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

static const LuaLReg Buffer_methods[] = {
  	{"new",           Buffer_new},
  	{"set_histsize",  Buffer_set_histsize},
  	{"get_histsize",  Buffer_get_histsize},
  	{"render",        Buffer_render},
  	{"print",         Buffer_print},
  	{"scroll",        Buffer_scroll},
  	{"scroll_to",     Buffer_scroll_to},
	{"format",        Buffer_format },
	{"format_escape", Buffer_format_escape},
  	{0, 0}
};

static int Buffer_gc (lua_State *L)
{
	Buffer *e = toBuffer(L, 1);
	buffer_free(e);
  	return 0;
}

static int Buffer_tostring (lua_State *L)
{
  	char buff[32];
  	sprintf(buff, "%p", toBuffer(L, 1));
  	lua_pushfstring(L, "Buffer (%s)", buff);
  	return 1;
}

static const LuaLReg Buffer_meta[] = {
  	{"__gc",       Buffer_gc},
  	{"__tostring", Buffer_tostring},
  	{0, 0}
};

void modBuffer_register (lua_State *L)
{
  	luaL_register(L, BUFFER, Buffer_methods); /* create methods table, add it to
  											   the globals */
  	lua_pushstring(L, BUFFER_INDENT_MARK_UTF);
  	lua_setfield(L, -2, "INDENT_MARK");

  	luaL_newmetatable(L, BUFFER);          /* create metatable for Buffer, and add
  											 it to the Lua registry */
  	luaL_openlib(L, 0, Buffer_meta, 0);    /* fill metatable */
  	  	lua_pushliteral(L, "__index");
  	lua_pushvalue(L, -3);               /* dup methods table*/
  	lua_rawset(L, -3);                  /* metatable.__index = methods */
  	lua_pushliteral(L, "__metatable");
  	lua_pushvalue(L, -3);               /* dup methods table*/
  	lua_rawset(L, -3);                  /* hide metatable:
                                           metatable.__metatable = methods */
  	lua_pop(L, 2);                      /* drop metatable and methods */
}
