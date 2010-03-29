#ifndef __MOONSHINE_LUA_H__
#define __MOONSHINE_LUA_H__
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <string.h>

#include <moonshine/config.h>

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

void ms_lua_backref_set(LuaState *L, gpointer ptr, int idx);
void ms_lua_backref_push(LuaState *L, gpointer ptr);
void ms_lua_backref_unset(LuaState *L, gpointer ptr);

INLINE gboolean ms_lua_checkboolean(LuaState *L, int index)
{
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index);
}

INLINE gboolean ms_lua_call(LuaState *L, int argc, int retc, const char *where)
{
    if (lua_pcall(L, argc, retc, 0)) {
        g_critical("moonshine error in %s: %s", where, lua_tostring(L, -1));
        return FALSE;
    }
    else {
        return TRUE;
    }
}

#endif
