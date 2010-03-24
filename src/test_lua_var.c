#include <glib.h>
#include "moonshine/lua.h"
#include "moonshine/lua_var.h"

int main()
{
    LuaState *L = ms_lua_newstate();
    lua_pushstring(L, "pants");
    lua_pushstring(L, "foobar");
    MSLuaVar *V = ms_lua_var_new(L, -1);
    lua_pop(L, 1);

    g_assert(strcmp(lua_tostring(L, -1), "pants") == 0);
    LuaState *L2 = ms_lua_var_push(V);
    g_assert(strcmp(lua_tostring(L, -1), "foobar") == 0);
    g_assert(L == L2);

    g_assert(V->count == 1);
    ms_lua_var_ref(V);
    g_assert(V->count == 2);
    ms_lua_var_unref(V);
    g_assert(V->count == 1);
    ms_lua_var_unref(V);
    lua_close(L);

    return 0;
}
