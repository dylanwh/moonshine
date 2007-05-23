#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <glib.h>
#include <lua.h>

gboolean moon_call(lua_State *L, const char *name, const char *sig, ...)
{
	int argc = strlen(sig);
    va_list vl;
    lua_checkstack(L, argc + 1);

    lua_getglobal(L, name);  /* get function */
    if (lua_isnil(L, -1))
    	return FALSE;

    va_start(vl, sig);
    /* push arguments */
    while (*sig)
    	switch (*sig++) {
          	case 'd':  /* double argument */
            	lua_pushnumber(L, va_arg(vl, double));
            	break;
          	case 'i':  /* int argument */
            	lua_pushnumber(L, va_arg(vl, int));
            	break;
          	case 's':  /* string argument */
            	lua_pushstring(L, va_arg(vl, char *));
            	break;
          	
          	default:
          		g_error("invalid option (%c)", *(sig - 1));
          		break;
        }
    /* do the call */
    if (lua_pcall(L, argc, 0, 0) != 0)
    	g_error("error running function `%s': %s",
    			name, lua_tostring(L, -1));
    va_end(vl);
    return TRUE;
}

void moon_export(lua_State *L, const char *name, lua_CFunction func, guint nargs)
{
	lua_pushcclosure(L, func, nargs); // adds a closure to the stack.
	lua_setglobal(L, name); // removes the closure from the stack
}
