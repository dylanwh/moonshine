#include "moonshine/config.h"
#include "moonshine/ms-lua.h"
#include <glib.h>

MSLuaRef *ms_lua_ref(lua_State *L, int idx)
{
	MSLuaRef *R = NULL;
	lua_pushvalue(L, idx);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	g_return_val_if_fail(ref != LUA_REFNIL, NULL);

	R = g_new0(MSLuaRef, 1);
	R->L = L;
	R->ref = ref;
	return ref;
}

void ms_lua_pushref(MSLuaRef *R)
{
	lua_rawgeti(R->L, LUA_REGISTRYINDEX, R->ref);
}

void ms_lua_unref(MSLuaRef *R)
{
	luaL_unref(R->L, LUA_REGISTRYINDEX, R->ref);
}

gpointer ms_lua_toclass(lua_State *L, const char *class, int index)
{
  	gpointer p = lua_touserdata(L, index);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer ms_lua_checkclass(lua_State *L, const char *class, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	gpointer p = luaL_checkudata(L, index, class);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer ms_lua_newclass(lua_State *L, const char *class, gsize size)
{
  	gpointer p = lua_newuserdata(L, size);
  	luaL_getmetatable(L, class);
  	lua_setmetatable(L, -2);
  	return p;
}


void ms_lua_class_register(lua_State *L, const char *class, const luaL_reg methods[], const luaL_reg meta[])
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
