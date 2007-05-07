#include <slang.h>
#include <ctype.h>
#include <stdlib.h>

#include "common.h"
#include "ui.h"


@implementation SpoonUI

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
