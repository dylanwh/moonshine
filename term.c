/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 : */
#include "term.h"
#include "config.h" // for BUFFER_COLOR_(MIN|MAX)_UCS
#include <string.h>
#include <stdlib.h>

static GHashTable *term_colors = NULL;
static int last_id = 0;

static unsigned char utf8_length[256] =
{
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 31 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 63 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 95 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 127 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 159 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 191 */
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* - 223 */
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1   /* - 255 */
};

static void term_reset(void)
{
	SLsmg_reset_smg ();
	SLang_reset_tty ();
	g_hash_table_destroy(term_colors);
}

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
	g_hash_table_insert(term_colors, g_strdup("default"), GINT_TO_POINTER(last_id++));
	g_hash_table_insert(term_colors, g_strdup("inverse"), GINT_TO_POINTER(last_id++));
	term_color_set("bob", "red", "black");
	term_color_set("topic", "lightgray", "blue");
	
	atexit(term_reset);
}

gunichar term_getkey(void)
{
	int ch = SLang_getkey();
	g_assert(ch <= 256);

	int len = utf8_length[ch];
	g_assert(len != 0);

	char buf[7];
	memset(buf, 0, sizeof buf);

	int i = 0;
	buf[i++] = (const char) ch;

	while (i < len)
		buf[i++] = (const char) SLang_getkey();

	g_assert(g_utf8_validate(buf, -1, NULL));
	return g_utf8_get_char(buf);
}

void term_resize(void)
{
	SLtt_get_screen_size();
	SLsmg_reinit_smg();
}

/* Color related functions */
void term_color_set(const char *name, const char *fg, const char *bg)
{
	g_assert(term_colors);
	last_id += 1;
	g_hash_table_insert(term_colors, g_strdup(name), GINT_TO_POINTER(last_id));
	SLtt_set_color( last_id, (char *)name, (char *) fg, (char *)bg);
}

void term_color_use(const char *name)
{
	SLsmg_set_color(term_color_to_id(name));
}

int term_color_to_id(const char *name) {
	g_assert(term_colors);
	gpointer color = g_hash_table_lookup(term_colors, name);
	if (color)
		return GPOINTER_TO_INT(color);
	else
		return 0;
}

const char *term_color_to_utf8(const char *name)
{
	/* Per g_unichar_to_utf8 docs we need 6 chars *
	 * here; add one for NUL					  */
	static char buf[7];

	gunichar ch = BUFFER_COLOR_MIN_UCS + term_color_to_id(name);
	g_assert(ch <= BUFFER_COLOR_MAX_UCS); /* XXX: handle this failure better... */

	gint len = g_unichar_to_utf8(ch, buf);
	g_assert(len < sizeof buf);
	buf[len] = 0;

	return buf;
}
