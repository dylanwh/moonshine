#ifndef __MOONSHINE_LUA_H__
#define __MOONSHINE_LUA_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

typedef lua_State     LuaState;
typedef luaL_reg      LuaLReg;
typedef lua_CFunction LuaFunction;

LuaState *ms_lua_newstate(void);

void ms_lua_stash_set(LuaState *L, const char *name, gpointer user_data);
gpointer ms_lua_stash_get(LuaState *L, const char *name);

void ms_lua_class_register(LuaState *L,
                           const char *name,
                           const LuaLReg methods[],
                           const LuaLReg meta[]);

gpointer ms_lua_toclass(LuaState *L,    const char *name, int index);
gpointer ms_lua_checkclass(LuaState *L, const char *name, int index);
gpointer ms_lua_newclass(LuaState *L,   const char *name, gsize size);

gboolean ms_lua_require(LuaState *L, const char *name);
void ms_lua_preload(LuaState *L, const char *name, LuaFunction func);
void ms_lua_use_env(LuaState *L);
void ms_lua_module(LuaState *L, const LuaLReg functions[]);

#endif
