#include <slang.h>
#include <stdlib.h>
#include <gnet.h>
#include <glib.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"
#include "string.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


Screen *screen;
Keyboard *keyboard;

static gboolean on_input(GIOChannel *input, GIOCondition cond, UNUSED gpointer data)
{
	if (cond & G_IO_IN) {
		const char *s = spoon_keyboard_read(keyboard);
		if (!s) return TRUE;
		if (s[1] == '\0')

		return TRUE;
	} else {
		g_print("stdin error!");
		return FALSE;
	}
}

static gboolean on_idle(gpointer data)
{
	if (spoon_term_resized()) {
		Screen *scr = (Screen *)data;
		spoon_screen_refresh(scr);
	}
	return TRUE;
}


int main(int argc, char *argv[])
{
	/* Set data */
	GMainLoop  *loop  = g_main_loop_new(NULL, TRUE);
	GIOChannel *input = g_io_channel_unix_new (fileno(stdin));
	keyboard = spoon_keyboard_new();
	screen   = spoon_screen_new();
	spoon_screen_refresh(screen);
	lua_State *L      = lua_open();
	luaL_openlibs(L);

	g_idle_add(on_idle, screen);
	g_io_add_watch(input, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, NULL);

	g_main_loop_run(loop);
	return 0;
}
