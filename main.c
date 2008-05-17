#include "moonshine.h"

int main(int argc, char *argv[])
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	if (luaL_dofile(L, "moonshine.lua") != 0) {
		g_error("%s", lua_tostring(L, -1));
	}
	return 0;
}
