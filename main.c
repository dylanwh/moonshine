/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"
#include "entry.h"

#include <string.h>
#include <ctype.h>

void on_signal(int fd, short event, void *arg)
{
	printf("signal: %d\n", fd);
	event_loopexit(NULL);
}

void on_input(int fd, short event, void *arg)
{
	Entry *entry = arg;
	gunichar c = term_getkey();
	entry_key(entry, c);
	entry_render(entry, 2);
	term_refresh();
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

	entry = entry_new();
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

	event_set(&input, fileno(stdin), EV_READ|EV_PERSIST, on_input, entry);
	event_add(&input, NULL);

	event_dispatch();

	term_reset();
	return 0;
}
