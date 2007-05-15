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
	scr->buffer = buffer_new(100);
	scr->entry  = g_string_new("");
	scr->entry_start = 0;
	scr->entry_pos   = 0;
	return scr;
}

void screen_free(Screen *scr)
{
	g_string_free(scr->topic, TRUE);
	buffer_free(scr->buffer);
	g_string_free(scr->entry, TRUE);
	g_free(scr);
}

void screen_refresh(Screen *scr)
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(scr->topic->str, scr->topic->len);

	/* render the buffer */
    buffer_render(scr->buffer);

	/* show entry text */
	SLsmg_gotorc(SLtt_Screen_Rows - 1, 0);
	SLsmg_write_nstring(scr->entry->str, SLtt_Screen_Cols);
	SLsmg_gotorc(SLtt_Screen_Rows - 1, scr->entry->len);

    /* finally, write to the real display */
	SLsmg_refresh();
}

void screen_print(Screen *scr, GString *msg)
{
    buffer_print(scr->buffer, msg);
}

void screen_enter(Screen *scr)
{
	screen_print(scr, scr->entry);
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

