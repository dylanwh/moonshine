/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __SPOON_ENTRY_H__
#define __SPOON_ENTRY_H__
#include <glib.h>
#include <lua.h>

typedef struct Entry Entry

Entry *entry_new(lua_State *L);

void entry_free(Entry *);

#endif

