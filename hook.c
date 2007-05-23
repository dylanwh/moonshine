#include <glib.h>
#include <lua.h>

void hook_setup(lua_State *L)
{
	lua_newtable(L);
	lua_setglobal(L, "hooks");
}

void hook_invoke(lua_State *L, const char *name, guint nargs)
{
	lua_getglobal(L, "hooks");
	lua_getfield(L, -1, name);
	lua_remove(L, -2); // remove "hooks" from the stack
	lua_insert(L, 1);  // move value from hooks[name] to the bottom (first element) of the stack.
}

void hook_add(lua_State *L, const char *name, lua_CFunction func, guint nargs)
{
	lua_getglobal(L, "hooks"); // adds "hooks" to the stack
	lua_pushcclosure(L, func, nargs); // adds a closure to the stack.
	lua_setfield(L, -2, name); // removes the closure from the stack
	lua_remove(L, -1);         // remove "hooks" from the stack
}

