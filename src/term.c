#include "moonshine/term.h"
#include <string.h>

/* for sig action: */
#include <signal.h>

/* for atexit() */
#include <stdlib.h>

static GHashTable *ms_term_colors = NULL;
static int last_id = 0;

static unsigned char utf8_length[256] =/*{{{*/
{
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 31 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 63 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 95 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 127 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 159 */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 191 */
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* - 223 */
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1   /* - 255 */
};/*}}}*/

void ms_term_reset(void)/*{{{*/
{
    static gboolean did_reset = FALSE;
    if (!did_reset) {
        SLsmg_reset_smg ();
        SLang_reset_tty ();
        g_hash_table_destroy(ms_term_colors);
    }
    did_reset = TRUE;
}/*}}}*/

static void on_abort(UNUSED int sig)/*{{{*/
{
    ms_term_reset();
    exit(1);
}/*}}}*/

void ms_term_init(void)/*{{{*/
{
    SLtt_get_terminfo ();
    g_assert(SLang_init_tty (0, 1, 1) != -1);
    // SLang_set_abort_signal(on_abort);
    SLsmg_init_smg ();
    SLutf8_enable (-1);
    SLsmg_utf8_enable(-1);
    SLsmg_refresh();
    /* SLsmg_embedded_escape_mode(1); */

    ms_term_colors = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(ms_term_colors, g_strdup("default"), GINT_TO_POINTER(last_id++));
    g_hash_table_insert(ms_term_colors, g_strdup("inverse"), GINT_TO_POINTER(last_id++));


    /* we try very hard to leave your terminal as we found it! */
    static struct sigaction sa;
    sa.sa_handler = on_abort;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, NULL);

    atexit(ms_term_reset);
}/*}}}*/

gunichar ms_term_getkey(void)/*{{{*/
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
}/*}}}*/

PURE int ms_term_charwidth(gunichar ch)/*{{{*/
{
    if (g_unichar_iswide(ch) || g_unichar_iscntrl(ch))
        return 2;
    else
        return 1;
}/*}}}*/

void ms_term_resize(void)/*{{{*/
{
    SLtt_get_screen_size();
    SLsmg_reinit_smg();
}/*}}}*/

/* Color related functions *//*{{{*/
void ms_term_color_set(const char *name, const char *fg, const char *bg)/*{{{*/
{
    g_return_if_fail(ms_term_colors);
    int *color_idp = g_hash_table_lookup(ms_term_colors, name);
    if (color_idp) {
        int color_id = GPOINTER_TO_INT(color_idp);
        SLtt_set_color( color_id, (char *)name, (char *) fg, (char *)bg);
    } else {
        last_id += 1;
        g_hash_table_insert(ms_term_colors, g_strdup(name), GINT_TO_POINTER(last_id));
        SLtt_set_color( last_id, (char *)name, (char *) fg, (char *)bg);
    }
}
/*}}}*/

void ms_term_color_use(const char *name)/*{{{*/
{
    SLsmg_set_color(ms_term_color_to_id(name));
}/*}}}*/

int ms_term_color_to_id(const char *name) {/*{{{*/
    g_return_val_if_fail(ms_term_colors, 0);
    gpointer color = g_hash_table_lookup(ms_term_colors, name);
    if (color)
        return GPOINTER_TO_INT(color);
    else
        return 0;
}/*}}}*/

const char *ms_term_color_to_utf8(const char *name)/*{{{*/
{
    /* Per g_unichar_to_utf8 docs we need 6 chars *
     * here; add one for NUL                      */
    static char buf[7];

    gunichar ch = MS_TERM_COLOR_MIN_UCS + ms_term_color_to_id(name);
    g_assert(ch <= MS_TERM_COLOR_MAX_UCS); /* XXX: handle this failure better... */

    gint len = g_unichar_to_utf8(ch, buf);
    g_assert(len < (gint)(sizeof buf));
    buf[len] = 0;

    return buf;
}/*}}}*/
/*}}}*/
