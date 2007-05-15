#ifndef _SPOON_UI_H
#define _SPOON_UI_H
#include <glib.h>
#include "buffer.h"

typedef struct {
	GString *topic;
	Buffer *buffer;
	GString *entry;
	guint entry_start;
	guint entry_pos;
} Screen;



Screen *screen_new(void);
void screen_free(Screen *scr);

void screen_print(Screen *scr, GString *msg);

void screen_refresh(Screen *scr);

void screen_addchar(Screen *scr, char c);
void screen_backspace(Screen *scr);
void screen_enter(Screen *scr);

gboolean screen_on_idle(gpointer data);

#endif
