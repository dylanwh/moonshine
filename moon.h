#ifndef __MOONSHINE_MOON_H__
#define __MOONSHINE_MOON_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

typedef lua_State    LuaState;
typedef luaL_reg     LuaLReg;

LuaState *moon_new(void);
gboolean moon_call(LuaState *L, const char *name, const char *sig, ...);
void moon_class_create(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[]);
void moon_weaktable(LuaState *L);
#define moon_require(L, package) moon_call(L, "require", "s", package)
#define moon_deref(L, ref) lua_rawgeti(L, LUA_REGISTRYINDEX, ref)
#define moon_unref(L, ref) luaL_unref(L, LUA_REGISTRYINDEX, ref)

static inline int moon_ref(LuaState *L, int idx)
{
	lua_pushvalue(L, idx);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

#endif
