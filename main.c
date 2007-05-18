#include <slang.h>
#include <stdlib.h>
#include <gnet.h>
#include <glib.h>
#include <ctype.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"
#include "signal.h"
#include "moon.h"
#include "closure.h"

int main(int argc, char *argv[])
{	
	// Screen *scr       = screen_new();
	// Keyboard *kb      = keyboard_new();
	//lua_State *L      = lua_open();
	//term_init();
	Moon *moon = moon_new();
	signal_init(moon);

	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);

	//keyboard_define(kb, "\r", "ENTER");
	//keyboard_define(kb, "^x", "EXIT");

	void on_exit(gpointer loop, UNUSED gpointer arg)
	{
		g_main_loop_quit((GMainLoop *)loop);
	}
	Closure *exit_c = closure_new(on_exit, loop, NULL);

	//keyboard_bind(kb, "EXIT", on_exit, loop);
	//signal_catch(SIGINT);
	signal_catch(SIGTERM);
	//signal_catch(SIGHUP);

	moon_bind(moon, "signal SIGTERM", exit_c);

	//screen_refresh(scr);
	g_main_loop_run(loop);

	//signal_reset();
	//term_reset();
	return 0;
}
