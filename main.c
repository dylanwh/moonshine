//#include <glib.h>
//#include <gnet.h>
#include <slang.h>
#include <stdlib.h>
#include <gnet.h>
#include <glib.h>

#include "config.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"

static gboolean on_input(GIOChannel *input, GIOCondition cond, gpointer data)
{
	if (cond & G_IO_IN) {
		Keyboard *kb = (Keyboard *)data;
		Key k = spoon_keyboard_read(kb);
		SLsmg_gotorc(0, 0);
		switch (k.type) {
			case KEY_TYPE_CHAR:
				SLsmg_printf("char = %c\n", k.data.c);
				break;
			case KEY_TYPE_NAME:
				SLsmg_printf("name = %s\n", k.data.name);
				break;
			case KEY_TYPE_ERROR:
				SLsmg_printf("error: %s\n", k.data.name);
				break;
		}
		SLsmg_refresh();
		return TRUE;
	} else {
		g_print("stdin error!");
		return FALSE;
	}
}

int main(int argc, char *argv[])
{
	spoon_term_init();
	GMainLoop  *loop   = g_main_loop_new(NULL, TRUE);
	GIOChannel *input  = g_io_channel_unix_new (fileno(stdin));
	Keyboard *kb      = spoon_keyboard_new();
	spoon_keyboard_define(kb, "^X", "control-x");

	g_io_add_watch(input, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, kb);
	g_main_loop_run(loop);
	return 0;
}
