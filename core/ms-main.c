#include <stdio.h>
#include <stdlib.h>
#include "moonshine/ms-lua.h"
#include "moonshine/ms-signal.h"
#include "moonshine/config.h"

static void init_paths(LuaState *L);

int main(int argc, char *argv[])
{
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	LuaState *L     = lua_open();
	luaL_openlibs(L);

	g_thread_init(NULL);
	init_paths(L);
	ms_signal_init();

	lua_createtable(L, argc, 1);
	lua_pushstring(L, argv[0]);
	lua_setfield(L, -2, "name");
	for (int i = 1; i < argc; i++) {
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i);
	}
	lua_setglobal(L, "argv");

	ms_lua_require(L, "moonshine.prelude");
	ms_lua_require(L, "moonshine");
	ms_lua_call_hook(L, "startup_hook");
	g_main_loop_run(loop);
	ms_lua_call_hook(L, "shutdown_hook");

	lua_close(L);
	g_main_loop_unref(loop);

	exit(0);
}

static void init_paths(LuaState *L)
{
	const char *runtime = g_getenv("MOONSHINE_RUNTIME");
	const char *modules = g_getenv("MOONSHINE_MODULES");

	if (!runtime)
		runtime = MOONSHINE_RUNTIME;

	if (!modules)
		modules = MOONSHINE_MODULES;

	/* push the global package onto the stack */
	lua_getglobal(L, "package");

	/* Assign package.path = runtime */
	lua_pushstring(L, runtime);
	lua_setfield(L, -2, "path");

	/* Assign package.cpath = modules */
	lua_pushstring(L, modules);
	lua_setfield(L, -2, "cpath");

	/* remove package from the stack. */
	lua_pop(L, 1);
}
