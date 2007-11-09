/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_TERM_H__
#define __MOONSHINE_TERM_H__
#include <glib.h>
#include <slang.h>

void term_init_colors(void);
void term_init(void);
void term_resize(void);

void term_color_set(const char *name, const char *fg, const char *bg);
void term_color_use(const char *name);
int term_color_to_id(const char *name);
const char *term_color_to_utf8(const char *name);
gunichar term_getkey(void);

#define term_color_use_id(id) SLsmg_set_color(id)

#define term_refresh    SLsmg_refresh
#define term_goto       SLsmg_gotorc
#define TERM_COLS       SLtt_Screen_Cols 
#define TERM_LINES      SLtt_Screen_Rows
#define term_erase_eol  SLsmg_erase_eol
#define term_write_gunichar SLsmg_write_char
#define term_write_chars(s) SLsmg_write_nchars(s, strlen(s))
#define term_write_chars_to(u, umax) SLsmg_write_chars(u, umax)
#define term_input_pending SLang_input_pending

#endif
