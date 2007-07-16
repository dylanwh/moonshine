/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"
#include "entry.h"
#include "buffer.h"

#include <signal.h>

struct Screen {
	gchar *topic; ///< The first line of the screen.
	Buffer *buffer; ///< Where chat messages pile up.
	Entry *entry;   ///< the text the user types into the client.
	Event sigwinch;
};

void on_signal_winch(int fd, short event, void *arg)
{
	Screen *scr = arg;
	term_resize();
	screen_refresh(scr);
}

Screen *screen_new(void)
{
	Screen *scr = emalloc(sizeof(Screen));

	scr->topic  = "<topic>";
	scr->buffer = buffer_new(100);
	scr->entry  = entry_new();

	event_set(&scr->sigwinch, SIGWINCH, EV_SIGNAL|EV_PERSIST, on_signal_winch, scr);
	event_add(&scr->sigwinch, NULL);

	return scr;
}

void screen_refresh(Screen *scr)
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(scr->topic, SLtt_Screen_Cols);

	/* render the buffer */
	buffer_render(scr->buffer);

	/* show entry text */
	entry_render(scr->entry, 0);

	/* finally, write to the real display */
	SLsmg_refresh();
}
