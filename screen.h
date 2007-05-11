#ifndef _SPOON_UI_H
#define _SPOON_UI_H
#include <glib.h>

typedef struct {
	GString *topic;
	GList *buffer;
	GString *entry;
	guint entry_start;
	guint entry_pos;
} Screen;



Screen *spoon_screen_new(void);
void spoon_screen_free(Screen *scr);

void spoon_screen_print(Screen *scr, GString *msg);

void spoon_screen_refresh(Screen *scr);

void spoon_screen_addchar(Screen *scr, char c);
void spoon_screen_backspace(Screen *scr);


#endif
