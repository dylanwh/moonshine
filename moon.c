#include "moon.h"i
#include "config.h"
#include <glib.h>

#define moon_ccall(L, f) (lua_pushcfunction(L, f), lua_call(L, 0, 0))

#ifdef EMBED_LUA
#	include <stdlib.h>

#	include "packages.h"
void moon_loader_init(LuaState *L);
#endif

int luaopen_Entry(LuaState *L);
int luaopen_Buffer(LuaState *L);
int luaopen_Topic(LuaState *L);
int luaopen_app(LuaState *L);
int luaopen_net(LuaState *L);

LuaState *moon_new(void)
{
	LuaState *L = lua_open();
	luaL_openlibs(L);
	moon_ccall(L, luaopen_Buffer);
	moon_ccall(L, luaopen_Entry);
	moon_ccall(L, luaopen_Topic);
	moon_ccall(L, luaopen_net);
//	moon_ccall(L, luaopen_Client);

#	ifdef EMBED_LUA
	moon_loader_init(L);
#	endif
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
          	case 'n': lua_pushnil(L); break;
          	default:  g_error("invalid option (%c)", *(sig - 1)); break;
        }
    if (lua_pcall(L, argc, 0, 0) != 0)
    	g_warning("error running function `%s': %s",
    			name, lua_tostring(L, -1));
    va_end(vl);
    return TRUE;
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

void moon_weaktable(LuaState *L)
{
	lua_newtable(L);               /* push: table */
	lua_newtable(L);               /* push: metatable */
	lua_pushstring(L, "v");        /* push: "v" */
	lua_setfield(L, -2, "__mode"); /* pop: "v" */
	lua_setmetatable(L, -2);       /* pop: metatable */
}

#ifdef EMBED_LUA
static int package_loader(LuaState *L)
{
	const char *pkg = lua_tostring(L, lua_upvalueindex(1));
	const char *src = lua_tostring(L, lua_upvalueindex(2));
	if (luaL_loadbuffer(L, src, strlen(src), pkg)) {
		const char *err = lua_tostring(L, -1);
		g_error("BUG: Cannot load in-core %s.lua: %s", pkg, err);
		exit(EXIT_FAILURE);
	} 
	else if (lua_pcall(L, 0, LUA_MULTRET, 0)) {
		const char *err = lua_tostring(L, -1);
		g_error("BUG: Boot from in-core %s.lua failed after pcall: %s", pkg, err);
		exit(EXIT_FAILURE);
	} else {
		return 0;
	}
}

static int package_finder(LuaState *L)
{
	const char *pkg = luaL_checkstring(L, 1);
	for (int i = 0; i < sizeof packages; i++) {
		if (strcmp(packages[i].filename, pkg) == 0) {
			lua_pushstring(L, pkg);
			lua_pushstring(L, packages[i].content);
			lua_pushcclosure(L, package_loader, 2);
			return 1;
		}
	}
	return 0;
}

void moon_loader_init(LuaState *L)
{
	/* table.insert(package.loaders, package_finder) */
	lua_register(L, "package_finder", package_finder);
	if (luaL_dostring(L, "table.insert(package.loaders, package_finder)")) {
		const char *err = lua_tostring(L, -1);
		g_error("BUG: error adding package loader: %s", err);
		exit(EXIT_FAILURE);
	}
	lua_pushnil(L);
	lua_setglobal(L, "package_finder");
}
#endif
