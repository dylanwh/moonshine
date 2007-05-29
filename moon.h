/** Hook Module. */
/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_MOON_H__
#define __MOONSHINE_MOON_H__

#include <glib.h>
#include <lua.h>

gboolean moon_call(lua_State *L, const char *name, const char *sig, ...);
gpointer moon_get_userdata(lua_State *L, gchar *name);

#endif
