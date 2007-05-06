//#include <glib.h>
//#include <gnet.h>
#include <slang.h>
#include <stdlib.h>

#include <gc.h>
#include <glib.h>

#include "protocol.h"
#include "hacks.h"

int main(int argc, char *argv[])
{
	hacks_init();
	while (1) {
		GSList *msg = haver_parse("foo\tbar\tbaz");
		GString *str = haver_format(msg);
		printf("%s\n", str->str);
		GC_gcollect();
	}
	return 0;
}

/*	SLtt_get_terminfo ();
	if (SLkp_init() == -1) {
	    SLang_doerror ("SLkp_init failed.");
	    exit (1);
	}
	if (SLang_init_tty (-1, 0, 1) == -1) {
	    SLang_doerror ("SLang_init_tty failed.");
	    exit (1);
	}
	SLsmg_init_smg ();	*/

/*	SLsmg_reset_smg ();
	SLang_reset_tty ();*/

