/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include <glib.h>
#include <slang.h>
#include <stdlib.h>

static GHashTable *term_colors;
static int last_id = 0;

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

	term_colors = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(term_colors, "default", GINT_TO_POINTER(last_id++));
	g_hash_table_insert(term_colors, "inverse", GINT_TO_POINTER(last_id++));
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
	g_hash_table_destroy(term_colors);
}

/* Color related functions */

void term_color_set(gchar *name, gchar *fg, gchar *bg)
{
	g_assert(term_colors);
	last_id += 1;
	g_hash_table_insert(term_colors, name, GINT_TO_POINTER(last_id));
	SLtt_set_color( last_id, name, fg, bg);
}

void term_color_use(gchar *name)
{
	g_assert(term_colors);
	gpointer color = g_hash_table_lookup(term_colors, name);
	if (color)
		SLsmg_set_color(GPOINTER_TO_INT(color));
	else
		SLsmg_set_color(0);
}

