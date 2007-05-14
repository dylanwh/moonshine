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

int main(int argc, char *argv[])
{	
	signal_init();
	term_init();

	Screen *scr       = screen_new();
	Keyboard *kb      = keyboard_new();
	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);

	keyboard_define(kb, "\r", "ENTER");
	keyboard_define(kb, "^x", "EXIT");
	void on_exit(gpointer loop, UNUSED gpointer arg)
	{
		g_main_loop_quit((GMainLoop *)loop);
	}

	Closure *exit_c = closure_new(on_exit, loop, NULL);

	keyboard_bind(kb, "EXIT", exit_c);
	signal_catch(SIGINT, exit_c);
	signal_catch(SIGTERM, exit_c);
	signal_catch(SIGHUP, exit_c);

	closure_unref(exit_c);

	screen_refresh(scr);
	g_main_loop_run(loop);

	term_reset();
	signal_reset();
	return 0;
}
