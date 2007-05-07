#include <slang.h>
#include <ctype.h>
#include <stdlib.h>

#include "common.h"
#include "screen.h"

@implementation Screen
- init
{
	topic     = g_string_new("");
	buffer    = NULL; // GList of GStrings
	entry     = g_string_sized_new(512);
	[self draw];
	return self;
}

- (void) free
{
	void each(GString *s, UNUSED gpointer unused) { g_string_free(s, TRUE); }
	g_string_free(topic, TRUE);
	g_string_free(entry, TRUE);
	g_list_foreach(buffer, (GFunc)each, NULL);
	g_list_free(buffer);
}

- (void) key_other: (int) key
{
	if (isprint((char)key))
		g_string_append_c(entry, (char)key);
	[self draw];
}

- (void) key_enter
{
	g_string_erase(entry, 0, entry->len);
	[self draw];
}

- (void) key_backspace
{
	g_string_truncate(entry, entry->len - 1);
	[self draw];
}

- (void) doesNotRecognize: (SEL)msg
{
	return;
}

- (void) draw
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(topic->str, topic->len);

	/* show entry text */
	SLsmg_gotorc(SLtt_Screen_Rows - 1, 0);
	SLsmg_write_nstring(entry->str, entry->len);

	/* finally, write to the real display */
	SLsmg_refresh();
}

@end
