#ifndef __MOONSHINE_MOON_H__
#define __MOONSHINE_MOON_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

void ms_lua_class_register(lua_State *L, const char *class, const luaL_reg methods[], const luaL_reg meta[]);
void ms_lua_weaktable(lua_State *L);

typedef struct MSLuaRef {
	lua_State *L;
	int ref;
} MSLuaRef;

MSLuaRef *ms_lua_ref(lua_State *L, int idx);
void ms_lua_pushref(MSLuaRef *R);
void ms_lua_unref(MSLuaRef *R);

gpointer ms_lua_toclass(lua_State *L, const char *class, int index);
gpointer ms_lua_checkclass(lua_State *L, const char *class, int index);
gpointer ms_lua_newclass(lua_State *L, const char *class, gsize size);
void ms_lua_pusherror(lua_State *L, GError *err);


#endif
