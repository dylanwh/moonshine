#ifndef __MOONSHINE_MOON_H__
#define __MOONSHINE_MOON_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

typedef lua_State LuaState;
typedef luaL_reg  LuaLReg;
typedef luaL_Buffer LuaLBuffer;

LuaState *moon_new(const char *path);
gboolean moon_call(LuaState *L, const char *name, const char *sig, ...);
void moon_class_register(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[]);
void moon_weaktable(LuaState *L);
#define moon_require(L, package) moon_call(L, "require", "s", package)
#define moon_pushref(L, ref) lua_rawgeti(L, LUA_REGISTRYINDEX, ref)
#define moon_unref(L, ref) luaL_unref(L, LUA_REGISTRYINDEX, ref)

int moon_ref(LuaState *L, int idx);

gpointer moon_toclass(LuaState *L, const char *class, int index);
gpointer moon_checkclass(LuaState *L, const char *class, int index);
gpointer moon_newclass(LuaState *L, const char *class, gsize size);
void moon_pusherror(LuaState *L, GError *err);

#endif
