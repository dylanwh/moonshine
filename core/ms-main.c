#include <stdio.h>
#include "moonshine/ms-lua.h"
#include "moonshine/config.h"

int main(int argc, char *argv[])
{
	const char *runtime = g_getenv("MOONSHINE_RUNTIME");
	const char *modules = g_getenv("MOONSHINE_MODULES");
	LuaState *L   = lua_open();
	luaL_openlibs(L);

	if (!runtime)
		runtime = MOONSHINE_RUNTIME;

	if (!modules)
		modules = MOONSHINE_MODULES;

	lua_getglobal(L, "package");
	lua_pushstring(L, runtime);
	lua_setfield(L, -2, "path");
	lua_pushstring(L, modules);
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	luaL_dofile(L, argv[1]);

	return 0;
}
