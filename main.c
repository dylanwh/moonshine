/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"
#include "entry.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void on_signal(int fd, short event, void *arg)
{
	printf("signal: %d\n", fd);
	event_loopexit(NULL);
}

void on_input(int fd, short event, void *arg)
{
	LuaState *L = arg;
	do {
		gunichar c = term_getkey();
		char buf[8];
		g_unichar_to_utf8(c, buf);
		moon_call(L, "on_input", "s", buf);
	} while (SLang_input_pending(-20));

	moon_call(L, "on_input_reset", "");
}

/* make_keyspec turns strings like "^A" into "\001", and so on. */
int make_keyspec(LuaState *L)
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

int main(int argc, char *argv[])
{
	Event sigint;
	Event sigterm;
	Event input;
	Entry *entry;
	LuaState *L;

	event_init();
	term_init();
	atexit(term_reset);

	entry = entry_new();
	L = moon_init();
	lua_pushcfunction(L, make_keyspec);
	lua_setglobal(L, "make_keyspec");

	//keymap_init(L);
	if (luaL_dofile(L, "lua/boot.lua")) {
		const char *err = lua_tostring(L, -1);
		g_error("Cannot boot: %s", err);
	}

	event_set(&sigint, SIGINT, EV_SIGNAL|EV_PERSIST, on_signal, NULL);
	event_add(&sigint, NULL);

	event_set(&sigterm, SIGTERM, EV_SIGNAL|EV_PERSIST, on_signal, NULL);
	event_add(&sigterm, NULL);

	event_set(&input, fileno(stdin), EV_READ|EV_PERSIST, on_input, L);
	event_add(&input, NULL);

	event_dispatch();

	//term_reset();
	return 0;
}
