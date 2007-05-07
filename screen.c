
//#include <glib.h>
//#include <gnet.h>
#include <slang.h>
#include <stdlib.h>

#include <glib.h>
#include "protocol.h"

static volatile gboolean size_changed = FALSE;
static void slang_reset(void);
static void slang_on_resize(int sig);
static inline gboolean slang_has_resized(void);

static void slang_init(void)
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
	SLsignal (SIGWINCH, slang_on_resize);
	SLsmg_init_smg ();
	atexit(slang_reset);
}

static slang_on_resize(int sig)
{
	slang_size_changed = TRUE;
	SLsignal (SIGWINCH, slang_on_resize);
}

static inline gboolean slang_resized(void)
{
	if (slang_size_changed) {
		slang_size_changed = FALSE;
		SLtt_get_screen_size();
		SLsmg_reinit_smg();
		return TRUE;
	} else
		return FALSE;
}

static void slang_reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
}
