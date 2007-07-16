/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 : */
#include <glib.h>
#include <slang.h>
#include <stdlib.h>
#include "buffer.h"
#include "term.h"
#include "moonshine.h"

//static int last_id = 0;

void term_init(void)
{
	SLtt_get_terminfo ();
	g_assert(SLang_init_tty (0, 1, 1) != -1);
	// SLang_set_abort_signal(on_abort);
	SLsmg_init_smg ();
	SLutf8_enable (-1);
	SLsmg_utf8_enable(-1);
	SLsmg_refresh();
	/* SLsmg_embedded_escape_mode(1); */

	//g_hash_table_insert(term_colors, "default", GINT_TO_POINTER(last_id++));
	//g_hash_table_insert(term_colors, "inverse", GINT_TO_POINTER(last_id++));
}


void term_resize(void)
{
	SLtt_get_screen_size();
	SLsmg_reinit_smg();
}

void term_reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
}
