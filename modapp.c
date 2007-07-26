#include "moonshine.h"
#include <string.h>
#define APP "app"

/* make_keyspec turns strings like "^A" into "\001", and so on. */
static int app_make_keyspec(LuaState *L)
{
	const char *str = luaL_checkstring(L, 1);
	int len = strlen(str);
	GString *buf = g_string_sized_new(len);

	for (int i = 0; i < len; i++) {
		if (str[i] == '^') {
			g_string_append_c(buf, str[i + 1] ^ 64);
			i++;
		} else {
			g_string_append_c(buf, str[i]);
		}
	}
	lua_pushstring(L, buf->str);
	g_string_free(buf, TRUE);
	return 1;
}

static int app_refresh(LuaState *L)
{
	term_refresh();
	return 0;
}

static int app_exit(LuaState *L)
{
	event_loopexit(NULL);
	return 0;
}

static LuaLReg functions[] = {
	{ "make_keyspec", app_make_keyspec },
	{ "exit", app_exit },
	{ "refresh", app_refresh },
	{ 0, 0 }
};

void modapp_register(LuaState *L)
{
	luaL_register(L, APP, functions);
	lua_pop(L, 1);
}
