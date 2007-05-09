#include <slang.h>
#include <stdlib.h>
#include <glib.h>
#include <signal.h>

// Private variables
static volatile gboolean resized = FALSE;

// Private Functions
static void reset(void);
static void on_resize(int sig);

void spoon_term_init(void)
{
	SLtt_get_terminfo ();
	g_assert(SLkp_init() != -1);
	g_assert(SLang_init_tty (-1, 0, 1) != -1);
	SLsignal (SIGWINCH, on_resize);
	SLsmg_init_smg ();
	atexit(reset);
}

inline gboolean spoon_term_resized(void)
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
