#include <slang.h>
#include <stdlib.h>
#include <gnet.h>
#include <glib.h>
#include <ctype.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"
#include "signal.h"

int main(int argc, char *argv[])
{	
	lua_State *L      = lua_open();
	//Screen *scr       = screen_new(L);
	UNUSED Keyboard *kb = keyboard_new(L);

	term_init();
	signal_init(L);
	luaL_openlibs(L);

	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);

	int quit(lua_State *L)
	{
		void *loop = lua_touserdata(L, lua_upvalueindex(1));
		g_main_loop_quit((GMainLoop *)loop);
		return 0;
	}
	lua_pushlightuserdata(L, loop);
	lua_pushcclosure(L, quit, 1);
	lua_setglobal(L, "quit");

	(void)luaL_dofile(L, "lua/boot.lua");

	signal_catch(SIGINT);
	signal_catch(SIGTERM);
	signal_catch(SIGHUP);

	//screen_refresh(scr);
	g_main_loop_run(loop);

	lua_close(L);
	signal_reset();
	term_reset();
	g_print("bye!\n");
	return 0;
}
