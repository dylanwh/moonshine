#include "moonshine.h"
#include "entry.h"

#include <string.h>
#define ENTRY "Entry"

static Entry *toEntry (lua_State *L, int index)
{
  	Entry **e = lua_touserdata(L, index);
  	if (e == NULL) luaL_typerror(L, index, ENTRY);
  	return *e;
}

static Entry *checkEntry (lua_State *L, int index)
{
  	Entry **e;
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	e = (Entry **)luaL_checkudata(L, index, ENTRY);
  	if (e == NULL) luaL_typerror(L, index, ENTRY);
  	return *e;
}

static Entry *pushEntry (lua_State *L)
{
  	Entry **e = (Entry **)lua_newuserdata(L, sizeof(Entry *));
  	luaL_getmetatable(L, ENTRY);
  	lua_setmetatable(L, -2);
  	*e = entry_new();
  	return *e;
}

static int Entry_new (lua_State *L)
{
  	pushEntry(L);
  	return 1;
}

static int Entry_keypress(lua_State *L)
{
  	Entry *e = checkEntry(L, 1);
  	const char *c = luaL_checkstring(L, 2);
  	gunichar uc = g_utf8_get_char(c);
  	entry_keypress(e, uc);
  	return 0;
}

static int Entry_move(LuaState *L)
{
	Entry *e = checkEntry(L, 1);
	int offset = luaL_checkinteger(L, 2);
	entry_move(e, offset);
	return 0;
}

static int Entry_move_to(LuaState *L)
{
	Entry *e     = checkEntry(L, 1);
	int absolute = luaL_checkinteger(L, 2);
	entry_move_to(e, absolute);
	return 0;
}

static int Entry_get(LuaState *L)
{
	Entry *e = checkEntry(L, 1);
	char *s = entry_get(e);
	lua_pushstring(L, s);
	g_free(s);
	return 1;
}

static int Entry_clear(LuaState *L)
{
	Entry *e = checkEntry(L, 1);
	entry_clear(e);
	return 0;
}

static int Entry_erase(LuaState *L)
{
	Entry *e = checkEntry(L, 1);
	int count = luaL_checkinteger(L, 2);
	entry_erase(e, count);
	return 0;
}


static int Entry_render(LuaState *L)
{
	Entry *e = checkEntry(L, 1);
	guint lmargin = luaL_optint(L, 2, 0);
	entry_render(e, lmargin);
	term_refresh();
	return 0;
}

static const LuaLReg Entry_methods[] = {
  	{"new",           Entry_new},
  	{"keypress",      Entry_keypress},
  	{"move",          Entry_move},
  	{"move_to",       Entry_move_to},
  	{"get",           Entry_get},
  	{"clear",         Entry_clear},
  	{"erase",         Entry_erase},
  	{"render",        Entry_render},
  	{0, 0}
};

static int Entry_gc (lua_State *L)
{
	Entry *e = toEntry(L, 1);
	entry_free(e);
  	return 0;
}

static int Entry_tostring (lua_State *L)
{
  	char buff[32];
  	sprintf(buff, "%p", toEntry(L, 1));
  	lua_pushfstring(L, "Entry (%s)", buff);
  	return 1;
}

static const LuaLReg Entry_meta[] = {
  	{"__gc",       Entry_gc},
  	{"__tostring", Entry_tostring},
  	{0, 0}
};

void modEntry_register (lua_State *L)
{
  	luaL_register(L, ENTRY, Entry_methods); /* create methods table, add it to
  											   the globals */
  	luaL_newmetatable(L, ENTRY);          /* create metatable for Entry, and add
  											 it to the Lua registry */
  	luaL_openlib(L, 0, Entry_meta, 0);    /* fill metatable */
  	lua_pushliteral(L, "__index");
  	lua_pushvalue(L, -3);               /* dup methods table*/
  	lua_rawset(L, -3);                  /* metatable.__index = methods */
  	lua_pushliteral(L, "__metatable");
  	lua_pushvalue(L, -3);               /* dup methods table*/
  	lua_rawset(L, -3);                  /* hide metatable:
                                           metatable.__metatable = methods */
  	lua_pop(L, 2);                      /* drop metatable and methods */
}
