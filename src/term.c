#include "moonshine/term.h"
#include <string.h>

/* for sig action: */
#include <signal.h>

/* for atexit() */
#include <stdlib.h>

static void on_abort(UNUSED int sig)
{
    ms_term_reset();
    exit(1);
}

void ms_term_init(void)
{
    initscr();
    raw();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, FALSE);

    /* we try very hard to leave your terminal as we found it! */
    static struct sigaction sa;
    sa.sa_handler = on_abort;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, NULL);

    atexit(ms_term_reset);
}

void ms_term_reset(void)
{
    endwin();
}


static unsigned char utf8_length[256] = {
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 31 */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 63 */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 95 */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 127 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 159 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 191 */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* - 223 */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1   /* - 255 */
};

/* FIXME: I really should be able to use get_wch.
 * But for some reason that doesn't work. */
gboolean ms_term_getkey(gunichar *rv)
{
    int ch = getch();
    g_return_val_if_fail(ch != ERR, FALSE);
    g_return_val_if_fail(ch <= 255, FALSE);

    int len = utf8_length[ch];
    g_return_val_if_fail(len != 0, FALSE);

    char buf[7];
    memset(buf, 0, sizeof buf);

    int i = 0;
    buf[i++] = (const char) ch;

    while (i < len)
        buf[i++] = (const char) getch();

    g_return_val_if_fail(g_utf8_validate(buf, -1, NULL), FALSE);
    *rv = g_utf8_get_char(buf);

    return TRUE;
}

PURE int ms_term_charwidth(gunichar ch)
{
    if (g_unichar_iswide(ch) || g_unichar_iscntrl(ch))
        return 2;
    else
        return 1;
}

void ms_term_resize(void)
{
    endwin();
    refresh();
}

void ms_term_write_gunichar(const gunichar ch)
{
    g_assert(sizeof(gunichar) == sizeof(wchar_t));
    cchar_t out;
    setcchar(&out, (wchar_t *) &ch, A_NORMAL, 0, NULL);
    add_wch(&out);
}

void ms_term_write_chars_to(const guchar *u, const guchar *umax)
{
    g_return_if_fail(u < umax);
    addnstr((const gchar *)u, (unsigned int) (umax - u));
}

void ms_term_color_set(UNUSED const char *name, UNUSED const char *fg, UNUSED const char *bg)
{
    /*
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
    */
}

void ms_term_color_use(UNUSED const char *name)
{
    //SLsmg_set_color(ms_term_color_to_id(name));
}

guint16 ms_term_color_to_id(UNUSED const char *name) {/*{{{*/
    /*
    g_return_val_if_fail(ms_term_colors, 0);
    gpointer color = g_hash_table_lookup(ms_term_colors, name);
    if (color)
        return GPOINTER_TO_INT(color);
    else
        return 0;
    */
    return 0;
}

void ms_term_color_use_id(UNUSED guint16 id)
{
}

const char *ms_term_color_to_utf8(const char *name)
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
}
