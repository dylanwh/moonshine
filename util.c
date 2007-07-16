/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */

#include "moonshine.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <gc.h>

void die(const char *errstr, ...) {
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void *emalloc(size_t size) {
	void *res = GC_malloc(size);

	if(!res)
		die("fatal: could not malloc() %u bytes\n", size);

	return res;
}

void efree(void *ptr)
{
	GC_free(ptr);
}

Event *event_new(int fd, short event, void (*fn)(int, short, void *), void *arg)
{
	Event *ev = emalloc(sizeof(Event));
	event_set(ev, fd, event, fn, arg);
	return ev;
}

LuaState *moon_init(void)
{
	LuaState *L = lua_open();
	luaL_openlibs(L);
	(void)luaL_dofile(L, "lua/boot.lua");
	return L;
}

bool moon_call(LuaState *L, const char *name, const char *sig, ...)
{
	int argc = strlen(sig);
    va_list vl;
    lua_checkstack(L, argc + 1);

    lua_getglobal(L, name);  /* get function */
    if (lua_isnil(L, -1))
    	return false;
    va_start(vl, sig);
    while (*sig)
    	switch (*sig++) {
          	case 'd': lua_pushnumber(L, va_arg(vl, double)); break;
          	case 'i': lua_pushnumber(L, va_arg(vl, int)); break;
          	case 's': lua_pushstring(L, va_arg(vl, char *)); break;
          	default:  die("invalid option (%c)", *(sig - 1)); break;
        }
    if (lua_pcall(L, argc, 0, 0) != 0)
    	die("error running function `%s': %s",
    			name, lua_tostring(L, -1));
    va_end(vl);
    return true;
}
