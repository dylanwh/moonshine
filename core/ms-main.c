#include <stdio.h>
#include "moonshine/ms-lua.h"

int main(int argc, char *argv[])
{
	LuaState *L = lua_open();
	luaL_openlibs(L);
	luaL_dofile(L, argv[1]);
	return 0;
}
