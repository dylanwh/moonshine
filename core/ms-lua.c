#include "moonshine/config.h"
#include "moonshine/ms-lua.h"
#include <glib.h>

MSLuaRef *ms_lua_ref(LuaState *L, int idx)
{
	MSLuaRef *R = NULL;
	lua_pushvalue(L, idx);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	g_return_val_if_fail(ref != LUA_REFNIL, NULL);

	R = g_new0(MSLuaRef, 1);
	R->L = L;
	R->ref = ref;
	return R;
}

void ms_lua_pushref(MSLuaRef *R)
{
	lua_rawgeti(R->L, LUA_REGISTRYINDEX, R->ref);
}

void ms_lua_unref(MSLuaRef *R)
{
	luaL_unref(R->L, LUA_REGISTRYINDEX, R->ref);
}

gpointer ms_lua_toclass(LuaState *L, const char *class, int index)
{
  	gpointer p = lua_touserdata(L, index);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer ms_lua_checkclass(LuaState *L, const char *class, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	gpointer p = luaL_checkudata(L, index, class);
  	if (p == NULL) luaL_typerror(L, index, class);
  	return p;
}

gpointer ms_lua_newclass(LuaState *L, const char *class, gsize size)
{
  	gpointer p = lua_newuserdata(L, size);
  	luaL_getmetatable(L, class);
  	lua_setmetatable(L, -2);
  	return p;
}

static void stackDump (lua_State *L) {
      int i;
      int top = lua_gettop(L);
      for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
    
          case LUA_TSTRING:  /* strings */
            printf("['%s']", lua_tostring(L, i));
            break;
    
          case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;
    
          case LUA_TNUMBER:  /* numbers */
            printf("[%g]", lua_tonumber(L, i));
            break; 
          default:  /* other values */
            printf("[%s]", lua_typename(L, t));
            break;
    
        }
        printf("  ");  /* put a separator */
      }
      printf("\n");  /* end the listing */
    }


void ms_lua_class_register(LuaState *L, const char *class, const LuaLReg methods[], const LuaLReg meta[])
{
  	luaL_register(L, class, methods); /* create methods table, add it to the
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
}


