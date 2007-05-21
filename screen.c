#include <slang.h>
#include <lua.h>
#include <lauxlib.h>

#include <ctype.h>
#include <stdlib.h>

#include "config.h"
#include "screen.h"
#include "term.h"

struct _Screen {
	GString *topic;
	Buffer *buffer;
	GString *entry;
	guint entry_start;
	guint entry_pos;
};

static int on_keypress(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	const char *s = luaL_checkstring(L, 1);
	g_assert(s);

	if (s != '\0' && s[1] == '\0')
		g_string_append_c(scr->entry, s[0]);
	else {
		char *name = g_strconcat("on_key_", s, NULL);
		lua_getglobal(L, name);
		if (!lua_isnil(L, -1))
			lua_call(L, 0, 0);
	}

	screen_refresh(scr);
	return 0;
}

static int on_key_backspace(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	g_string_truncate(scr->entry, scr->entry->len - 1);
	return 0;
}

static int on_key_enter(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	screen_print(scr, scr->entry);
	g_string_truncate(scr->entry, 0);
	return 0;
}

Screen *screen_new(lua_State *L)
{
	Screen *scr = g_new(Screen, 1);

	scr->topic  = g_string_new("");
	scr->buffer = buffer_new(100);
	scr->entry  = g_string_new("");
	scr->entry_start = 0;
	scr->entry_pos   = 0;


	lua_pushlightuserdata(L, scr);
	lua_pushcclosure(L, on_keypress, 1);
	lua_setglobal(L, "on_keypress");

	lua_pushlightuserdata(L, scr);
	lua_pushcclosure(L, on_key_backspace, 1);
	lua_setglobal(L, "on_key_backspace");

	lua_pushlightuserdata(L, scr);
	lua_pushcclosure(L, on_key_enter, 1);
	lua_setglobal(L, "on_key_enter");


	return scr;
}

void screen_free(Screen *scr)
{
	g_string_free(scr->topic, TRUE);
	buffer_free(scr->buffer);
	g_string_free(scr->entry, TRUE);
	g_free(scr);
}

void screen_refresh(Screen *scr)
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(scr->topic->str, scr->topic->len);

	/* render the buffer */
	buffer_render(scr->buffer);

	/* show entry text */
	SLsmg_gotorc(SLtt_Screen_Rows - 1, 0);
	SLsmg_write_nstring(scr->entry->str, SLtt_Screen_Cols);
	SLsmg_gotorc(SLtt_Screen_Rows - 1, scr->entry->len);

	/* finally, write to the real display */
	SLsmg_refresh();
}

void screen_print(Screen *scr, GString *msg)
{
	buffer_print(scr->buffer, msg);
}


