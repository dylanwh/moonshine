/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

LuaState *moon_init(void)
{
	LuaState *L = lua_open();
	luaL_openlibs(L);
	modapp_register(L);
	modEntry_register(L);
	modBuffer_register(L);
	return L;
}

gboolean moon_call(LuaState *L, const char *name, const char *sig, ...)
{
	int argc = strlen(sig);
    va_list vl;
    lua_checkstack(L, argc + 1);

    lua_getglobal(L, name);  /* get function */
    if (lua_isnil(L, -1))
    	return FALSE;
    va_start(vl, sig);
    while (*sig)
    	switch (*sig++) {
          	case 'd': lua_pushnumber(L, va_arg(vl, double)); break;
          	case 'i': lua_pushnumber(L, va_arg(vl, int)); break;
          	case 's': lua_pushstring(L, va_arg(vl, char *)); break;
          	default:  g_error("invalid option (%c)", *(sig - 1)); break;
        }
    if (lua_pcall(L, argc, 0, 0) != 0)
    	g_error("error running function `%s': %s",
    			name, lua_tostring(L, -1));
    va_end(vl);
    return TRUE;
}

PURE int unicode_charwidth(gunichar ch) {
	if (g_unichar_iswide(ch))
		return 2;
	else
		return 1;
}
