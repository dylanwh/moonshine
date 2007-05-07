//#include <glib.h>
//#include <gnet.h>
#include <slang.h>
#include <stdlib.h>

#include <glib.h>
#include "common.h"
#include "term.h"
#include "screen.h"
#include "keyboard.h"

static gboolean on_input(GIOChannel *input, GIOCondition cond, gpointer data)
{
	if (cond & G_IO_IN) {
		id kb = (id)data;
		[kb processKey];
		return TRUE;
	} else {
		g_print("stdin error!");
		return FALSE;
	}
}

int main(int argc, char *argv[])
{
	spoon_term_init();
	GMainLoop *loop   = g_main_loop_new(NULL, TRUE);
	GIOChannel *input = g_io_channel_unix_new (fileno(stdin));
	id screen   = [Screen new];
	id keyboard = [[Keyboard new] bind: screen];
	g_io_add_watch(input, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, keyboard);
	g_main_loop_run(loop);
	return 0;
}
