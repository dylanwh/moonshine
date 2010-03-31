/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

/* Moonshine - a Lua-based chat client
 *
 * Copyright (C) 2010 Dylan William Hardison
 *
 * This file is part of Moonshine.
 *
 * Moonshine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonshine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moonshine/term.h"
#include <string.h>

/* for sig action: */
#include <signal.h>

/* for atexit() */
#include <stdlib.h>

gushort ms_term_style = 0;
static GHashTable *ms_term_bold  = NULL;

INLINE void STYLE_MARK_BOLD(gushort style)
{
    g_assert(ms_term_bold);
    g_hash_table_insert(ms_term_bold, GUINT_TO_POINTER((gulong)style), GINT_TO_POINTER(1));
}
INLINE gboolean STYLE_IS_BOLD(gushort style)
{
    g_assert(ms_term_bold);
    return g_hash_table_lookup(ms_term_bold, GUINT_TO_POINTER((gulong)style)) != NULL;
}

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

    ms_term_bold = g_hash_table_new(g_direct_hash, g_direct_equal);
    atexit(ms_term_reset);
}

void ms_term_reset(void)
{
    static gboolean did_free = FALSE;
    if (!did_free && ms_term_bold != NULL) {
        g_hash_table_destroy(ms_term_bold);
        did_free = TRUE;
    }
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

    int flag = STYLE_IS_BOLD(ms_term_style) ? A_BOLD : A_NORMAL;
    setcchar(&out, buf, flag, ms_term_style, NULL);
    add_wch(&out);
}

void ms_term_style_set(gushort style)
{
    ms_term_style = style;
    color_set(style, NULL);
    if (STYLE_IS_BOLD(style))
        attron(A_BOLD);
    else
        attroff(A_BOLD);
}

void ms_term_style_init(gushort style, gushort fg, gushort bg)
{
    g_return_if_fail(style < MS_TERM_STYLES);
    switch (MS_TERM_COLORS) {
        case 8:
            init_pair(style, fg % 8, bg % 8);
            if (fg > 7)
                STYLE_MARK_BOLD(style);
            break;
        case 16:
        case 88:
        case 256:
            g_return_if_fail(fg < MS_TERM_COLORS);
            g_return_if_fail(bg < MS_TERM_COLORS);
            init_pair(style, fg, bg);
            break;
        default:
            g_assert_not_reached();
            break;
    }

}

void ms_term_color_init(gushort color, gushort r, gushort g, gushort b)
{
    g_return_if_fail(color < MS_TERM_COLORS);
    g_return_if_fail(r <= 1000);
    g_return_if_fail(g <= 1000);
    g_return_if_fail(b <= 1000);

    init_color(color, r, g, b);
}



const char *ms_term_style_code(gushort id)
{
    /* Per g_unichar_to_utf8 docs we need 6 chars *
     * here; add one for NUL                      */
    static char buf[7];

    gunichar ch = MS_TERM_STYLE_MIN_UCS + id;
    g_assert(ch <= MS_TERM_STYLE_MAX_UCS); /* XXX: handle this failure better... */

    gint len = g_unichar_to_utf8(ch, buf);
    g_assert(len < (gint)(sizeof buf));
    buf[len] = 0;

    return buf;
}
