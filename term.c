#include <slang.h>
#include <stdlib.h>
#include <glib.h>
#include <signal.h>

void spoon_term_init(void)
{
	SLtt_get_terminfo ();
	g_assert(SLang_init_tty (0, 1, 1) != -1);
/*	SLsignal (SIGWINCH, on_resize);
	SLsignal (SIGHUP, on_abort);
	SLsignal (SIGTERM, on_abort);
	SLang_set_abort_signal(on_abort);*/
	SLsmg_init_smg ();
}


void spoon_term_resize(void)
{
	SLtt_get_screen_size();
	SLsmg_reinit_smg();
}

void spoon_term_reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
	g_print("Goodbye!\n");
}
