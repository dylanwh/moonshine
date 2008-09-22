#include "moonshine/config.h"
#include "moonshine/ms-lua.h"
#include <glib.h>

/* MSLuaRef-related functions {{{ */
MSLuaRef *ms_lua_ref(LuaState *L, int idx)/*{{{*/
{
	MSLuaRef *R = NULL;
	lua_pushvalue(L, idx);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	g_return_val_if_fail(ref != LUA_REFNIL, NULL);

	R = g_new0(MSLuaRef, 1);
	R->L = L;
	R->ref = ref;
	return R;
}/*}}}*/

LuaState *ms_lua_pushref(MSLuaRef *R)/*{{{*/
{
	lua_rawgeti(R->L, LUA_REGISTRYINDEX, R->ref);
	return R->L;
}/*}}}*/

void ms_lua_unref(MSLuaRef *R)/*{{{*/
{
	luaL_unref(R->L, LUA_REGISTRYINDEX, R->ref);
}/*}}}*/
/* }}} */

/* Class-related functions {{{ */

gpointer ms_lua_toclass(LuaState *L, const char *class, int index)/*{{{*/
{
  	gpointer p = lua_touserdata(L, index);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}/*}}}*/

gpointer ms_lua_checkclass(LuaState *L, const char *class, int index)/*{{{*/
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	gpointer p = luaL_checkudata(L, index, class);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}/*}}}*/

gpointer ms_lua_newclass(LuaState *L, const char *class, gsize size)/*{{{*/
{
  	gpointer p = lua_newuserdata(L, size);
  	luaL_getmetatable(L, class);
  	lua_setmetatable(L, -2);
  	return p;
}/*}}}*/

void ms_lua_class_register(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[])/*{{{*/
{
	lua_newtable(L);
  	luaL_register(L, NULL, methods); /* create methods table, add it to the
  										 globals */

  	luaL_newmetatable(L, class);      /* create metatable for Buffer, and add it
  										 to the Lua registry */
  	luaL_register(L, NULL, meta);      /* fill metatable */

  	lua_pushliteral(L, "__index");    /* Set meta.__index = methods */
  	lua_pushvalue(L, -3);             /* dup methods table*/
  	lua_rawset(L, -3);                /* metatable.__index = methods */

  	lua_pushliteral(L, "__metatable");
  	lua_pushvalue(L, -3);             /* dup methods table*/
  	lua_rawset(L, -3);                /* hide metatable: metatable.__metatable = methods */
 
 	lua_remove(L, -1);
}/*}}}*/

/*}}}*/

void ms_lua_call_hook(LuaState *L, const char *name)/*{{{*/
{
	g_return_if_fail(L != NULL);
	g_return_if_fail(name != NULL);

	lua_getglobal(L, name);
	if (lua_isfunction(L, -1)) {
		if (lua_pcall(L, 0, 0, 0) != 0) {
			g_error("moonshine error in %s(): %s", name, lua_tostring(L, -1));
		}
	} else {
		lua_pop(L, 1);
	}
}/*}}}*/

void ms_lua_require(LuaState *L, const char *name)/*{{{*/
{
	g_return_if_fail(L != NULL);
	g_return_if_fail(name != NULL);

	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	if(lua_pcall(L, 1, 0, 0) != 0) {
		g_error("moonshine error in require '%s': %s", name, lua_tostring(L, -1));
	}
}/*}}}*/

