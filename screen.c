
//#include <glib.h>
//#include <gnet.h>
#include <slang.h>
#include <stdlib.h>
#include <glib.h>
#include <signal.h>

#include "protocol.h"

// Private variables
static volatile gboolean resized = FALSE;

// Private Functions
static void reset(void);
static void on_resize(int sig);

void haver_screen_init(void)
{
	SLtt_get_terminfo ();
	if (SLkp_init() == -1) {
	    SLang_doerror ("SLkp_init failed.");
	    exit (1);
	}
	if (SLang_init_tty (-1, 0, 1) == -1) {
	    SLang_doerror ("SLang_init_tty failed.");
	    exit (1);
	}
	SLsignal (SIGWINCH, on_resize);
	SLsmg_init_smg ();
	atexit(reset);
}

inline gboolean haver_screen_resized(void)
{
	if (resized) {
		resized = FALSE;
		SLtt_get_screen_size();
		SLsmg_reinit_smg();
		return TRUE;
	} else
		return FALSE;
}

static void on_resize(int sig)
{
	resized = TRUE;
	SLsignal (SIGWINCH, on_resize);
}

static void reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
}
