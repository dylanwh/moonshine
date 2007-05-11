#include <slang.h>
#include <stdlib.h>
#include <gnet.h>
#include <glib.h>
#include <ctype.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"
#include "string.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


int main(int argc, char *argv[])
{
	/* Set data */
	GMainLoop  *loop  = g_main_loop_new(NULL, TRUE);
	GIOChannel *input = g_io_channel_unix_new (fileno(stdin));
	Screen *scr       = spoon_screen_new();
	Keyboard *kb      = spoon_keyboard_new();


	void on_keypress(gpointer env, gpointer arg)
	{
		Screen *scr = (Screen *)env;
		char c = *(char *)arg;
		if (isprint(c))
			spoon_screen_addchar(scr, c);
		spoon_screen_refresh(scr);
	}

	void on_backspace(gpointer env, UNUSED gpointer arg)
	{
		Screen *scr = (Screen *)env;
		spoon_screen_backspace(scr);
		spoon_screen_refresh(scr);
	}

	void on_enter(gpointer env, UNUSED gpointer arg)
	{
		Screen *scr = (Screen *)env;
		spoon_screen_enter(scr);
		spoon_screen_refresh(scr);
	}
	void on_exit(UNUSED gpointer env, UNUSED gpointer arg)
	{
		exit(0);
	}

	spoon_keyboard_define(kb, "\r", "ENTER");
	spoon_keyboard_define(kb, "^x", "EXIT");
	spoon_keyboard_bind_fallback(kb, spoon_closure_new(on_keypress, scr, NULL));
	spoon_keyboard_bind(kb, "BACKSPACE", spoon_closure_new(on_backspace, scr, NULL));
	spoon_keyboard_bind(kb, "ENTER", spoon_closure_new(on_enter, scr, NULL));
	spoon_keyboard_bind(kb, "EXIT", spoon_closure_new(on_exit, NULL, NULL));

	//Closure *backspace = spoon_closure_new(on_backspace, screen, NULL);

	spoon_screen_refresh(scr);

	g_idle_add(spoon_screen_on_idle, scr);
	g_io_add_watch(input, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, spoon_keyboard_on_input, kb);

	g_main_loop_run(loop);
	return 0;
}
