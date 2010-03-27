#include "moonshine/term.h"
#include <string.h>

/* for sig action: */
#include <signal.h>

/* for atexit() */
#include <stdlib.h>

static guint16 ms_term_color = 0;

static void on_abort(UNUSED int sig)
{
    ms_term_reset();
    exit(1);
}

void ms_term_init(void)
{
    initscr();
    start_color();
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

gboolean ms_term_getkey(gunichar *rv)
{
#ifdef MOONSHINE_USE_GET_WCH
    int code = get_wch(rv);
    g_return_val_if_fail(code != ERR, FALSE);
#else
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
#endif
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
    cchar_t out;
    wchar_t buf[2];
    buf[0] = (wchar_t) ch;
    buf[1] = L'\0';

    setcchar(&out, buf, A_NORMAL, ms_term_color, NULL);
    add_wch(&out);
}

void ms_term_color_set(guint16 id)
{
    ms_term_color = id;
    color_set(id, NULL);
}

const char *ms_term_color_code(guint16 id)
{
    /* Per g_unichar_to_utf8 docs we need 6 chars *
     * here; add one for NUL                      */
    static char buf[7];

    gunichar ch = MS_TERM_COLOR_MIN_UCS + id;
    g_assert(ch <= MS_TERM_COLOR_MAX_UCS); /* XXX: handle this failure better... */

    gint len = g_unichar_to_utf8(ch, buf);
    g_assert(len < (gint)(sizeof buf));
    buf[len] = 0;

    return buf;
}
