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

int main(int argc, char *argv[])
{
	Event sigint;
	Event sigterm;
	Event input;
	LuaState *L;

	event_init();
	term_init();
	L = moon_init();
	
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

	term_reset();
	lua_close(L);
	return 0;
}
