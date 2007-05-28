/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_TERM_H__
#define __MOONSHINE_TERM_H__

void term_init(void);
void term_resize(void);
void term_reset(void);

void term_color_set(gchar *name, gchar *fg, gchar *bg);
void term_color_use(gchar *name);

#endif
