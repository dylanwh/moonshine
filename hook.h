/** Hook Module. */
/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __SPOON_HOOK_H__
#define __SPOON_HOOK_H__

#include <glib.h>
#include <lua.h>

/** Configure lua runtime for hooks. 
 * */
void hook_setup(lua_State *L);
void hook_invoke(lua_State *L, const char *name, guint nargs);
void hook_add(lua_State *L, const char *name, lua_CFunction func, guint nargs);

#endif
