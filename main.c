/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */

#include "moonshine.h"
#include "term.h"
#include "screen.h"
#include <ctype.h>

/* borrowed from slang */
static unsigned char lmap[256] =
{
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 31 */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 63 */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 95 */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  /* - 127 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* - 191 */
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  /* - 223 */
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1   /* - 255 */
};





void on_signal(int fd, short event, void *arg)
{
	printf("signal: %d\n", fd);
	event_loopexit(NULL);
}

void on_input(int fd, short event, void *arg)
{
	int c = SLang_getkey();
	SLsmg_gotorc(0, 0);

	if (isascii(c) && iscntrl(c)) {
		if (c == '\e') {
			if (SLang_input_pending(1)) {
				char key = SLang_getkey();
				SLsmg_printf("M-%c", key);
			} else {
				SLsmg_printf("esc");
			}
		} else {
			SLsmg_printf("C-%c", c ^ 64);
		}
	} else {
		unsigned char buf[7];
		unsigned char len = lmap[c];
		int pos = 1;

		buf[0] = (unsigned char)c;
		while (len-- > 1) {
			buf[pos++] = (unsigned char)SLang_getkey();
		}
		buf[pos] = '\0';
		SLsmg_printf("%s", buf);
	}
	SLsmg_refresh();
}

int main(int argc, char *argv[])
{
	Event sigint;
	Event sigterm;
	Event input;
	term_init();
	event_init();

	/* Initalize one event */
	event_set(&sigint, SIGINT, EV_SIGNAL|EV_PERSIST, on_signal, NULL);
	event_add(&sigint, NULL);

	event_set(&sigterm, SIGTERM, EV_SIGNAL|EV_PERSIST, on_signal, NULL);
	event_add(&sigterm, NULL);

	event_set(&input, fileno(stdin), EV_READ|EV_PERSIST, on_input, NULL);
	event_add(&input, NULL);

	event_dispatch();

	term_reset();
	return 0;
}
