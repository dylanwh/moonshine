#include <stdio.h>
#include <stdlib.h>
#include "moonshine/lua.h"
#include "moonshine/signal.h"
#include "moonshine/term.h"
#include "moonshine/config.h"


int luaopen_moonshine_loop_core(LuaState *);
int luaopen_moonshine_net_client(LuaState *);
int luaopen_moonshine_parseopt_core(LuaState *);
int luaopen_moonshine_tree(LuaState *);
int luaopen_moonshine_ui_buffer(LuaState *);
int luaopen_moonshine_ui_entry(LuaState *);
int luaopen_moonshine_ui_statusbar(LuaState *);
int luaopen_moonshine_ui_term(LuaState *);

int main(int argc, char *argv[])
{
	LuaState *L     = ms_lua_newstate();

	ms_lua_preload(L, "moonshine.loop.core",    luaopen_moonshine_loop_core);
	ms_lua_preload(L, "moonshine.net.client",   luaopen_moonshine_net_client);
	ms_lua_preload(L, "moonshine.parseopt.core",luaopen_moonshine_parseopt_core);
	ms_lua_preload(L, "moonshine.tree",         luaopen_moonshine_tree);
	ms_lua_preload(L, "moonshine.ui.buffer",    luaopen_moonshine_ui_buffer);
	ms_lua_preload(L, "moonshine.ui.entry",     luaopen_moonshine_ui_entry);
	ms_lua_preload(L, "moonshine.ui.statusbar", luaopen_moonshine_ui_statusbar);
	ms_lua_preload(L, "moonshine.ui.term",      luaopen_moonshine_ui_term);

	g_thread_init(NULL);
	ms_signal_init();
	ms_term_init();

	lua_getglobal(L, "require");
	lua_pushstring(L, "moonshine");
	if(lua_pcall(L, 1, 0, 0) != 0)
		g_warning("moonshine error in require 'moonshine': %s", lua_tostring(L, -1));

	ms_term_reset();
	ms_signal_reset();
	exit(0);
}
