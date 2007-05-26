/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include <slang.h>
#include <lua.h>
#include <lauxlib.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "screen.h"
#include "term.h"
#include "signal.h"
#include "moon.h"
#include "entry.h"

struct Screen {
	GString *topic; ///< The first line of the screen.
	Buffer *buffer; ///< Where chat messages pile up.
	Entry *entry;   ///< the text the user types into the client.
};

static int on_keypress(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	const char *s = luaL_checkstring(L, 1);
	lua_pop(L, -1);
	g_assert(s);

	/* XXX: We just take the first byte of S as a unicode codepoint. This is
	 *   very incorrect, but will do for now as a horrible hack. In the future,
	 *   keypress needs to deal with gunichars, or utf8 sequences.
	 */
	entry_key(scr->entry, s[0]);

	screen_refresh(scr);
	return 0;
}

static int on_key_backspace(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	entry_erase(scr->entry, -1);
	screen_refresh(scr);
	return 0;
}

static int on_key_delete(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	entry_erase(scr->entry, 1);
	screen_refresh(scr);
	return 0;
}

static int on_key_enter(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	gchar *str = entry_get(scr->entry);
	if (!strcmp(str, "/quit")) {
		/* XXX: until we have real command parsing */
		raise(SIGTERM); /* :D */
	} else {
		GString s; /* XXX: this is annoying */
		s.str = str;
		s.len = strlen(str);
		screen_print(scr, &s);
		g_free(s.str);
	}
	entry_clear(scr->entry);
	screen_refresh(scr);
	return 0;
}

static int on_key_left(lua_State *L) {
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	buffer_print(scr->buffer, "LEFT");
	entry_move(scr->entry, -1);
	screen_refresh(scr);
	return 0;
}

static int on_key_right(lua_State *L) {
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	buffer_print(scr->buffer, "RIGHT");
	entry_move(scr->entry, 1);
	screen_refresh(scr);
	return 0;
}


static int on_signal_SIGWINCH(lua_State *L)
{
	Screen *scr = lua_touserdata(L, lua_upvalueindex(1));
	term_resize();
	screen_refresh(scr);
	return 0;
}

Screen *screen_new(lua_State *L)
{
	Screen *scr = g_new(Screen, 1);

	scr->topic  = g_string_new("<topic>");
	scr->buffer = buffer_new(100);
	scr->entry  = entry_new(L);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_keypress", on_keypress, 1);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_key_backspace", on_key_backspace, 1);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_key_delete", on_key_delete, 1);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_key_enter", on_key_enter, 1);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_key_left", on_key_left, 1);

	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_key_right", on_key_right, 1);

	signal_catch(SIGWINCH);
	lua_pushlightuserdata(L, scr);
	moon_export(L, "on_signal_SIGWINCH", on_signal_SIGWINCH, 1);

	return scr;
}

void screen_free(Screen *scr)
{
	g_string_free(scr->topic, TRUE);
	buffer_free(scr->buffer);
	entry_free(scr->entry);
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
	entry_render(scr->entry, 0);

	/* finally, write to the real display */
	SLsmg_refresh();
}

void screen_print(Screen *scr, GString *msg)
{
	buffer_print(scr->buffer, msg->str);
}


