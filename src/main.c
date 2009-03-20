#include <stdio.h>
#include <stdlib.h>
#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/config.h"

int main(int argc, char *argv[])
{
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	LuaState *L     = ms_lua_newstate();

	//ms_lua_preload(L, "moonshine.loop", luaopen_moonshine_loop);

	g_thread_init(NULL);
	ms_signal_init();

	ms_signal_reset();
	exit(0);
}
