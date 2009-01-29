#include <stdio.h>
#include <stdlib.h>
#include "moonshine/ms-lua.h"
#include "moonshine/ms-signal.h"
#include "moonshine/config.h"

int main(int argc, char *argv[])
{
	LuaState *L     = ms_lua_open(argc, argv);

	g_thread_init(NULL);
	ms_signal_init();

	ms_lua_require(L, "moonshine");

	/* The next three function calls are terribly order-dependent. */
	ms_signal_reset();
	lua_close(L);

	exit(0);
}
