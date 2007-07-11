/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include <slang.h>
#include <lua.h>
#include <lauxlib.h>
#include <event.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "screen.h"
#include "term.h"
#include "moon.h"
#include "entry.h"

struct Screen {
	GString *topic; ///< The first line of the screen.
	Buffer *buffer; ///< Where chat messages pile up.
	Entry *entry;   ///< the text the user types into the client.
	struct event sigwinch;
};

void on_signal_winch(int fd, short event, void *arg)
{
	Screen *scr = arg;
	term_resize();
	screen_refresh(scr);
}

Screen *screen_new(lua_State *L)
{
	Screen *scr = lua_newuserdata(L, sizeof(Screen));
	lua_setglobal(L, "screen");

	scr->topic  = g_string_new("<topic>");
	scr->buffer = buffer_new(100);
	scr->entry  = entry_new(L);

	event_set(&scr->sigwinch, SIGWINCH, EV_SIGNAL|EV_PERSIST, on_signal_winch, scr);
	event_add(&scr->sigwinch, NULL);
	return scr;
}

void screen_refresh(Screen *scr)
{
	/* write the topic */
	term_color_use("topic");
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(scr->topic->str, SLtt_Screen_Cols);
	term_color_use("default");

	/* render the buffer */
	buffer_render(scr->buffer);

	/* show entry text */
	entry_render(scr->entry, 0);

	/* finally, write to the real display */
	SLsmg_refresh();
}

void screen_print(Screen *scr, GString *msg)
{
	buffer_print(scr->buffer, msg->str);
}


