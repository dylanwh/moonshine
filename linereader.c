#include "moon.h"

typedef struct {
	GString *buffer;
	gboolean iterating;
} LineReader;

static int LineReader_new(LuaState *L)
{
	LineReader *lr = moon_newclass(L, "LineReader", sizeof(LineReader));
	lr->buffer = g_string_new("");
	return 1;
}


static int readiter(LuaState *L)
{
	LineReader *lr = moon_checkclass(L, "LineReader", 1);
	GString *buffer = lr->buffer;
	if (buffer->len) {
		for (int i = 0; i < buffer->len; i++) {
			if (buffer->str[i] == '\n') {
				lua_pushlstring(L, buffer->str, i);
				g_string_erase(buffer, 0, i+1);
				g_main_context_iteration(NULL, FALSE);
				return 1;
			}
		}
	}
	return 0;
}

static int LineReader_read(LuaState *L)
{
	LineReader *lr  = moon_checkclass(L, "LineReader", 1);
	const char *str = luaL_checkstring(L, 2);
	gsize len       = lua_objlen(L, 2);

	GString *buffer = lr->buffer;
	g_string_append_len(buffer, str, len);
	lua_pushcfunction(L, readiter);
	lua_pushvalue(L, 1);
	return 2;
}

static int LineReader_gc(LuaState *L)
{
	LineReader *lr = moon_toclass(L, "LineReader", 1);
	g_string_free(lr->buffer, TRUE);
	return 0;
}

static int LineReader_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "LineReader", 1));
  	lua_pushfstring(L, "LineReader (%s)", buff);
  	return 1;
}

static const LuaLReg LineReader_methods[] = {
	{"new", LineReader_new},
	{"read", LineReader_read},
	{0, 0}
};

static const LuaLReg LineReader_meta[] = {
	{"__gc", LineReader_gc},
	{"__tostring", LineReader_tostring},
	{0, 0}
};

int luaopen_linereader(LuaState *L)
{
	moon_class_register(L, "LineReader", LineReader_methods, LineReader_meta);
	return 1;
}
