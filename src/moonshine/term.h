/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_MS_TERM_H__
#define __MOONSHINE_MS_TERM_H__

#include "moonshine/config.h"

#include <glib.h>
#include <slang.h>
#include <string.h>

#define MS_TERM_INDENT_MARK_UCS 0xF0000
#define MS_TERM_INDENT_MARK_UTF "\xF3\xB0\x80\x80"
#define MS_TERM_COLOR_MIN_UCS   0xFC000
#define MS_TERM_COLOR_MAX_UCS   0xFCFFF

void ms_term_init(void);
void ms_term_reset(void);
void ms_term_resize(void);

void ms_term_color_set(const char *name, const char *fg, const char *bg);
void ms_term_color_use(const char *name);
int ms_term_color_to_id(const char *name);
const char *ms_term_color_to_utf8(const char *name);
gunichar ms_term_getkey(void);

PURE int ms_term_charwidth(gunichar ch);

#define ms_term_color_use_id(id) SLsmg_set_color(id)
#define ms_term_refresh    SLsmg_refresh
#define ms_term_goto       SLsmg_gotorc
#define MS_TERM_COLS       SLtt_Screen_Cols 
#define MS_TERM_LINES      SLtt_Screen_Rows
#define ms_term_erase_eol  SLsmg_erase_eol
#define ms_term_write_gunichar SLsmg_write_char
#define ms_term_write_chars(s) SLsmg_write_nchars(s, strlen(s))
#define ms_term_write_chars_to(u, umax) SLsmg_write_chars(u, umax)
#define ms_term_input_pending SLang_input_pending

#endif
