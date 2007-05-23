#include <glib.h>
#include <lua.h>
#include <lauxlib.h>
#include "../hook.h"

int main(int argc, char *argv[])
{
	lua_State *L = lua_open();
	hook_setup(L);
	lua_getglobal(L, "hooks");
	g_assert(!lua_isnil(L, 1));
	lua_pop(L, 1);

	return 0;
}
