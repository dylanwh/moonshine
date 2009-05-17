#include <stdio.h>
#include <stdlib.h>
#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/config.h"

int luaopen_moonshine_loop(LuaState *);
int luaopen_moonshine_tree(LuaState *);
int luaopen_moonshine_parseopt_core(LuaState *);
int luaopen_moonshine_net_client(LuaState *);
int luaopen_moonshine_ui_buffer(LuaState *);
int luaopen_moonshine_ui_entry(LuaState *);
int luaopen_moonshine_ui_statusbar(LuaState *);
int luaopen_moonshine_ui_term(LuaState *);

int main(int argc, char *argv[])
{
	LuaState *L     = ms_lua_newstate();

	ms_lua_preload(L, "moonshine.loop",         luaopen_moonshine_loop);
	ms_lua_preload(L, "moonshine.tree",         luaopen_moonshine_tree);
	ms_lua_preload(L, "moonshine.parseopt.core",luaopen_moonshine_parseopt_core);
	ms_lua_preload(L, "moonshine.net.client",   luaopen_moonshine_net_client);
	ms_lua_preload(L, "moonshine.ui.buffer",    luaopen_moonshine_ui_buffer);
	ms_lua_preload(L, "moonshine.ui.entry",     luaopen_moonshine_ui_entry);
	ms_lua_preload(L, "moonshine.ui.statusbar", luaopen_moonshine_ui_statusbar);
	ms_lua_preload(L, "moonshine.ui.term",      luaopen_moonshine_ui_term);

	g_thread_init(NULL);
	ms_signal_init();

	ms_lua_require(L, "moonshine");

	ms_signal_reset();
	exit(0);
}
