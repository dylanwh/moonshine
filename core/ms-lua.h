#ifndef __MOONSHINE_MOON_H__
#define __MOONSHINE_MOON_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

typedef lua_State     LuaState;
typedef luaL_reg      LuaLReg;
typedef lua_CFunction LuaFunction;

LuaState *ms_lua_open(void);

typedef struct MSLuaRef {
	LuaState *L;
	int ref;
} MSLuaRef;

MSLuaRef *ms_lua_ref(LuaState *L, int idx);
LuaState *ms_lua_pushref(MSLuaRef *R);
void ms_lua_unref(MSLuaRef *R);

void ms_lua_class_register(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[]);
gpointer ms_lua_toclass(LuaState *L, const char *class, int index);
gpointer ms_lua_checkclass(LuaState *L, const char *class, int index);
gpointer ms_lua_newclass(LuaState *L, const char *class, gsize size);

void ms_lua_require(LuaState *L, const char *name);

#endif
