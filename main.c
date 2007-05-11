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
	/* Set data */
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

	void on_resize(UNUSED gpointer env, UNUSED gpointer arg)
	{
		spoon_term_resize();
	}

	Closure *exit_closure = spoon_closure_new(on_exit, NULL, NULL);
	Closure *fb_closure   = spoon_closure_new(on_keypress, scr, NULL);
	Closure *bs_closure   = spoon_closure_new(on_backspace, scr, NULL);
	Closure *enter_closure = spoon_closure_new(on_enter, scr, NULL);
	Closure *resize_closure = spoon_closure_new(on_resize, NULL, NULL);

	spoon_keyboard_define(kb, "\r", "ENTER");
	spoon_keyboard_define(kb, "^x", "EXIT");
	spoon_keyboard_bind_fallback(kb, fb_closure);

	spoon_keyboard_bind(kb, "BACKSPACE", bs_closure);
	spoon_keyboard_bind(kb, "ENTER", enter_closure);
	spoon_keyboard_bind(kb, "EXIT", exit_closure);

	spoon_screen_refresh(scr);
	spoon_signal_register(SIGINT, exit_closure);
	spoon_signal_register(SIGTERM, exit_closure);
	spoon_signal_register(SIGHUP, exit_closure);
	spoon_signal_register(SIGWINCH, resize_closure);

	spoon_signal_add_watch();
	spoon_keyboard_add_watch(kb);

	spoon_closure_unref(exit_closure);
	spoon_closure_unref(fb_closure);
	spoon_closure_unref(bs_closure);
	spoon_closure_unref(enter_closure);
	spoon_closure_unref(resize_closure);

	g_main_loop_run(g_main_loop_new(NULL, TRUE));
	return 0;
}

__attribute__((constructor)) static void pre_main(void)
{
	spoon_signal_init();
	spoon_term_init();
}

__attribute__((destructor)) static void post_main(void)
{
	spoon_term_reset();
	spoon_signal_reset();
}
