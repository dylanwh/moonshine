/*
    Moonshine - a Lua-based chat client
    
    Copyright (C) 2010 Dylan William Hardison
    
    This file is part of Moonshine.
    
    Moonshine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Moonshine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
*/

/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_MS_TERM_H__
#define __MOONSHINE_MS_TERM_H__

#include "moonshine/config.h"

#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>

#include <glib.h>
#include <string.h>

/* character used for indentation purposes in modui_buffer.c */
#define MS_TERM_INDENT_MARK_UCS  0xF0000
#define MS_TERM_INDENT_MARK_UTF "\xF3\xB0\x80\x80"

/* character used to encode "switch style" */
#define MS_TERM_STYLE_MIN_UCS    0xFC000
#define MS_TERM_STYLE_MAX_UCS    0xFCFFF

/* character to represent "use previous style" */
#define MS_TERM_STYLE_RESET_UCS  (MS_TERM_STYLE_MAX_UCS+1)
#define MS_TERM_STYLE_RESET_UTF "\xF3\xBD\x80\x80"

/* fake style to represent "use previous style" in modui_buffer.c */
#define MS_TERM_RESET_STYLE     (MS_TERM_STYLE_RESET_UCS - MS_TERM_STYLE_MIN_UCS)

#define MS_TERM_COLS       COLS
#define MS_TERM_LINES      LINES
#define MS_TERM_STYLES     COLOR_PAIRS
#define MS_TERM_COLORS     COLORS

void        ms_term_init(void);
void        ms_term_reset(void);
void        ms_term_resize(void);
gboolean    ms_term_getkey(gunichar *rv);
PURE int    ms_term_charwidth(gunichar ch);


INLINE void ms_term_refresh()   { refresh();  }
INLINE void ms_term_goto(int r, int c)      { move(r, c);     }
INLINE void ms_term_erase_eol() { clrtoeol(); }

extern gushort ms_term_style;
void        ms_term_style_set(gushort style);
const char *ms_term_style_code(gushort style);
void ms_term_style_init(gushort style, gushort fg, gushort bg);
void ms_term_color_init(gushort color, gushort r, gushort g, gushort b);


void        ms_term_write_gunichar(gunichar c);
INLINE int  ms_term_write_chars(const char *s) { return addstr(s); }
INLINE void ms_term_write_chars_to(const guchar *u, const guchar *umax)
{
    g_return_if_fail(u <= umax);
    addnstr((const gchar *)u, (unsigned int) (umax - u));
}

#endif
