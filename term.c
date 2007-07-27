/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 : */
#include "moonshine.h"

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

gunichar term_getkey(void)
{
	int ch = SLang_getkey();
	g_assert(ch <= 256);

	int len = utf8_length[ch];
	g_assert(len != 0);

	gchar buf[8];
	for (int j = 0; j < 8; j++)
		buf[j] = 0;

	int i = 0;
	buf[i++] = (gchar) ch;

	while (i < len)
		buf[i++] = (gchar) SLang_getkey();

	g_assert(g_utf8_validate(buf, -1, NULL));
	return g_utf8_get_char(buf);
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
