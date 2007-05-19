#ifndef __SPOON_SCREEN_H__
#define __SPOON_SCREEN_H__
#include <lua.h>
#include <glib.h>

#include "buffer.h"

typedef struct {
	GString *topic;
	Buffer *buffer;
	GString *entry;
	guint entry_start;
	guint entry_pos;
} Screen;



Screen *screen_new(lua_State *L);
void screen_free(Screen *scr);

void screen_print(Screen *scr, GString *msg);

void screen_refresh(Screen *scr);

void screen_addchar(Screen *scr, char c);
void screen_backspace(Screen *scr);
void screen_enter(Screen *scr);

gboolean screen_on_idle(gpointer data);

#endif
