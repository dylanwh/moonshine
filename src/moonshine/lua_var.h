#ifndef __MOONSHINE_LUA_VAR_H__
#define __MOONSHINE_LUA_VAR_H__
#include "moonshine/lua.h"

typedef struct MSLuaVar {
    LuaState *state;
    int slot;
    int count;
    gboolean use_env;
} MSLuaVar;

#define ms_lua_var_new(L, idx)            ms_lua_var_new_full(L, idx, LUA_TNONE, FALSE)
#define ms_lua_var_new_type(L, idx, type) ms_lua_var_new_full(L, idx, type, FALSE)

MSLuaVar *ms_lua_var_new_full(LuaState *L, int idx, int type, gboolean use_env);
LuaState *ms_lua_var_push(MSLuaVar *V);
void      ms_lua_var_free(MSLuaVar *V);
MSLuaVar *ms_lua_var_ref(MSLuaVar *V);
void      ms_lua_var_unref(MSLuaVar *V);

#endif
