#include <slang.h>
#include <ctype.h>
#include <stdlib.h>

#include "config.h"
#include "screen.h"
#include "term.h"

Screen *screen_new(void)
{
	Screen *scr = g_new(Screen, 1);
	scr->topic  = g_string_new("");
	scr->buffer = NULL; // GList of GStrings
	scr->entry  = g_string_new("");
	scr->entry_start = 0;
	scr->entry_pos   = 0;
	return scr;
}

void screen_free(Screen *scr)
{
	void each(GString *s, UNUSED gpointer unused) { g_string_free(s, TRUE); }
	g_string_free(scr->topic, TRUE);
	g_list_foreach(scr->buffer, (GFunc)each, NULL);
	g_list_free(scr->buffer);
	g_string_free(scr->entry, TRUE);
	g_free(scr);
}

void screen_refresh(Screen *scr)
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(scr->topic->str, scr->topic->len);

	/* show entry text */
	SLsmg_gotorc(SLtt_Screen_Rows - 1, 0);
	SLsmg_write_nstring(scr->entry->str, SLtt_Screen_Cols);
	SLsmg_gotorc(SLtt_Screen_Rows - 1, scr->entry->len);

	/* finally, write to the real display */
	SLsmg_refresh();
}

void screen_print(Screen *scr, GString *msg)
{
	scr->buffer = g_list_prepend(scr->buffer, (gpointer)msg);
}

void screen_enter(Screen *scr)
{
	screen_print(scr, g_string_new(scr->entry->str));
	g_string_truncate(scr->entry, 0);
}

void screen_addchar(Screen *scr, char c)
{
	g_string_append_c(scr->entry, c);
}

void screen_backspace(Screen *scr)
{
	g_string_truncate(scr->entry, scr->entry->len - 1);
}

