#include <slang.h>
#include <stdlib.h>
#include <glib.h>
#include <signal.h>


/* Private variables */
static volatile gboolean resized = FALSE;

/* Private Functions */
static void on_resize(int sig);
static void on_abort(int sig);

/* Our only public function! */
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

/* This calls all functions needed to initialize the terminal.
 * the __attribute__ GCC feature is used to ensure it is called before main(). */
__attribute__((constructor)) static void init(void)
{
	SLtt_get_terminfo ();
	g_assert(SLang_init_tty (0, 1, 1) != -1);
	SLsignal (SIGWINCH, on_resize);
	SLsignal (SIGHUP, on_abort);
	SLsignal (SIGTERM, on_abort);
	SLang_set_abort_signal(on_abort);
	SLsmg_init_smg ();
}

static void on_resize(int sig)
{
	resized = TRUE;
	SLsignal (SIGWINCH, on_resize);
}

/* FIXME: Is it safe to call exit(0) inside a signal handler? */
static void on_abort(int sig)
{
	exit(0);
}

/* This restores the terminal to its original state.
 * the __attribute__ GCC feature is used to ensure it is called after main().
 * We could more portably use atexit(), but this is more fun. */
__attribute__((destructor)) static void reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
	g_print("Goodbye!\n");
}
