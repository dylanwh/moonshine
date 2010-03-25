/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_MS_TERM_H__
#define __MOONSHINE_MS_TERM_H__

#include "moonshine/config.h"

#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>

#include <glib.h>
#include <string.h>

#define MS_TERM_INDENT_MARK_UCS 0xF0000
#define MS_TERM_INDENT_MARK_UTF "\xF3\xB0\x80\x80"
#define MS_TERM_COLOR_MIN_UCS   0xFC000
#define MS_TERM_COLOR_MAX_UCS   0xFCFFF

#define MS_TERM_COLS       COLS
#define MS_TERM_LINES      LINES

void        ms_term_init(void);
void        ms_term_reset(void);
void        ms_term_resize(void);
gboolean    ms_term_getkey(gunichar *rv);
PURE int    ms_term_charwidth(gunichar ch);
void        ms_term_write_gunichar(gunichar c);


INLINE void ms_term_refresh()   { refresh();  }
INLINE void ms_term_goto(int r, int c)      { move(r, c);     }
INLINE void ms_term_erase_eol() { clrtoeol(); }

void        ms_term_color_set(guint16 id);
const char *ms_term_color_to_utf8(guint16 id);
INLINE void ms_term_init_pair(guint16 id, guint16 fg, guint16 bg) { init_pair(id, fg, bg);  }
INLINE void ms_term_init_color(guint16 n, guint16 r, guint16 g, guint16 b) { init_color(n, r, g, b); }

INLINE int ms_term_write_chars(const char *s) { return addstr(s); }
INLINE void ms_term_write_chars_to(const guchar *u, const guchar *umax)
{
    g_return_if_fail(u <= umax);
    addnstr((const gchar *)u, (unsigned int) (umax - u));
}

#endif
