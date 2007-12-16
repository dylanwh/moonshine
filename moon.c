#include "moon.h"
#include "config.h"
#include <glib.h>

#include "moonlibs.h"

static void moon_openlibs(LuaState *L) {
  const LuaLReg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}

LuaState *moon_new(const char *path)
{
	LuaState *L = lua_open();
	moon_openlibs(L);
	
	lua_getglobal(L, "package");
	lua_pushstring(L, path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	lua_pushstring(L, VERSION);
	lua_setglobal(L, "VERSION");

	return L;
}

gboolean moon_call(LuaState *L, const char *name, const char *sig, ...)
{
	gboolean rv = TRUE;
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
          	case 'n': lua_pushnil(L); break;
          	default:  g_error("invalid option (%c)", *(sig - 1)); break;
        }
    if (lua_pcall(L, argc, 0, 0) != 0) {
    	rv = FALSE;
    	g_warning("error running function `%s': %s",
    			name, lua_tostring(L, -1));
    	lua_pop(L, 1);
    }
    va_end(vl);
    return rv;
}

int moon_ref(LuaState *L, int idx)
{
	lua_pushvalue(L, idx);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	g_assert(ref != LUA_REFNIL);
	return ref;
}

gpointer moon_toclass(LuaState *L, const char *class, int index)
{
  	gpointer p = lua_touserdata(L, index);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer moon_checkclass(LuaState *L, const char *class, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	gpointer p = luaL_checkudata(L, index, class);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer moon_newclass(LuaState *L, const char *class, gsize size)
{
  	gpointer p = lua_newuserdata(L, size);
  	luaL_getmetatable(L, class);
  	lua_setmetatable(L, -2);
  	return p;
}


void moon_class_register(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[])
{
  	luaL_register(L, class, methods); /* create methods table, add it to the
  										 globals */
  	luaL_newmetatable(L, class);      /* create metatable for Buffer, and add it
  										 to the Lua registry */
  	luaL_openlib(L, 0, meta, 0);      /* fill metatable */
  	lua_pushliteral(L, "__index");
  	lua_pushvalue(L, -3);             /* dup methods table*/
  	lua_rawset(L, -3);                /* metatable.__index = methods */
  	lua_pushliteral(L, "__metatable");
  	lua_pushvalue(L, -3);             /* dup methods table*/
  	lua_rawset(L, -3);                /* hide metatable: metatable.__metatable =
  										 methods */
  	lua_pop(L, 2);                    /* drop metatable and methods */
}

void moon_pusherror(LuaState *L, GError *err)
{
	lua_checkstack(L, 4);
	lua_createtable(L, 0, 3);
	lua_pushstring(L, g_quark_to_string(err->domain));
	lua_setfield(L, -2, "domain");
	lua_pushinteger(L, err->code);
	lua_setfield(L, -2, "code");
	lua_pushstring(L, err->message);
	lua_setfield(L, -2, "message");
	lua_pushstring(L, "Error");
	lua_setfield(L, -2, "__type");
}
