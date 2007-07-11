/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include <slang.h>
#include <stdlib.h>
#include <ctype.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <event.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"

int main(int argc, char *argv[])
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	event_init();
	term_init();

	(void)luaL_dofile(L, "lua/boot.lua");

	event_dispatch();

	lua_close(L);
	term_reset();
	g_print("bye!\n");
	return 0;
}
