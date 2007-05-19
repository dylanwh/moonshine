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
	Screen *scr       = screen_new(L);
	Keyboard *kb      = keyboard_new(L);
	signal_init(L);

	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);

	void quit(lua_State *lua)
	{
		g_main_loop_quit((GMainLoop *)loop);
	}
	Closure *exit_c = closure_new(on_exit, loop, NULL);

	signal_catch(SIGINT);
	signal_catch(SIGTERM);
	signal_catch(SIGHUP);

	//screen_refresh(scr);
	g_main_loop_run(loop);

	//signal_reset();
	//term_reset();
	return 0;
}
