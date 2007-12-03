/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "term.h"
#include "util.h"
#include "moon.h"

#include <string.h>

typedef struct {
	gchar *prompt;
	gunichar *buffer;
	gsize bufsize; /* The total size of the buffer in gunichars. Is zero iff buffer is NULL. */
	gsize bufused; /* The length of the actual string in the buffer. */
	gsize view_off; /* The index of the first visible character. May be out of range; entry_render will correct such issues. */
	gsize curs_off; /* The index of the character the cursor is on. Must not be out of range (> bufused). */
	int dirty; /* True if the entry has been modified since the last set(), clear(), or clear_dirty() */
} Entry;

static int Entry_new(LuaState *L)
{
	const char *prompt = luaL_optstring(L, 2, "[moonshine] ");
	Entry *e = moon_newclass(L, "Entry", sizeof(Entry));
	e->buffer = NULL;
	e->bufsize = e->bufused = 0;
	e->view_off = e->curs_off = 0;
	e->prompt   = g_strdup(prompt);
	e->dirty	= 0;
	return 1;
}

static int Entry_set_prompt(LuaState *L)
{
	Entry *e           = moon_checkclass(L, "Entry", 1);
	const char *prompt = luaL_checkstring(L, 2);
	g_free(e->prompt);
	e->prompt = g_strdup(prompt);
	return 0;
}

inline static int keypress(Entry *e, gunichar uc)
{
	if (!uc || !g_unichar_isdefined(uc))
		return 0; /* Filter invalid characters, hopefully. XXX: is this enough to deny the PUA? */
	if (e->bufused + 1 > e->bufsize) {
		/* If e->bufused * sizeof(e->buffer[0]) > 2**32, then there's a theoretical heap overflow on Really Big Systems.
		 * Let's just clamp it to something nice and sane like 16kilochars
		 */
		if (e->bufsize >= 16384) return 0;
		/* We assume most lines will be relatively small, so just increase by 128 each time */
		e->bufsize += 128;
		e->buffer = g_renew(gunichar, e->buffer, e->bufsize);
	}
	memmove(e->buffer + e->curs_off + 1, e->buffer + e->curs_off,
			sizeof(e->buffer[0]) * (e->bufused - e->curs_off));
	e->buffer[e->curs_off] = uc;
	e->bufused++;
	e->curs_off++;
	e->dirty = 1;
	return 0;
}

static int Entry_keypress(LuaState *L)
{
	Entry *e        = moon_checkclass(L, "Entry", 1);
	const char *key = luaL_checkstring(L, 2);
	gunichar uc = g_utf8_get_char(key);

	return keypress(e, uc);
}

static int Entry_move(LuaState *L)
{
	Entry *e   = moon_checkclass(L, "Entry", 1);
	int offset = luaL_checkinteger(L, 2);  

	if (offset == 0) return 0;
	
	gsize new_off = e->curs_off + offset;
	if ((new_off < e->curs_off) != (offset < 0)) {
		/* Integer overflow has occured. Move the the end or start. */
		if (offset < 0)
			new_off = 0;
		else
			new_off = e->bufused;
	}
	if (new_off > e->bufused)
		new_off = e->bufused;
	e->curs_off = new_off;
	return 0;
}

static int Entry_move_to(LuaState *L)
{
	Entry *e     = moon_checkclass(L, "Entry", 1);
	int absolute = luaL_checkinteger(L, 2);  

	if (!e->bufused) return 0;

	if (absolute >= 0) {
		e->curs_off = MIN(e->bufused, absolute);
	} else { /* -1 is after the last char, -2 on the last, etc... */
		/* Note: If absolute is too far negative, we'll get gsize wraparound here. */
		e->curs_off = e->bufused + absolute + 1;
		if (e->curs_off > e->bufused)
			e->curs_off = 0;
	}
	return 0;
}

static int Entry_get(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);
	if (e->bufused == 0) {
		lua_pushstring(L, "");
		return 1;
	} else {
		char *str = g_ucs4_to_utf8(e->buffer, e->bufused, NULL, NULL, NULL);
		lua_pushstring(L, str);
		g_free(str);
		return 1;
	}
}



inline static int clear(Entry *e)
{
	e->bufused = 0;
	e->curs_off = e->view_off = 0;
	/* If we have more than a page of buffer, free it, to prevent a single
	 * zomg-large string from using memory forever.
	 */
	if (e->bufused > 1024) {
		g_free(e->buffer);
		e->bufused = 0;
	}
	e->dirty = 0;
	return 0;
}

static int Entry_clear(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);
	return clear(e);
}

static int Entry_set(LuaState *L)
{
	Entry *e         = moon_checkclass(L, "Entry", 1);
	const char *line = luaL_checkstring(L, 2);

	e->dirty = 0;
	GError *error;
	glong written;
	gunichar *buffer = g_utf8_to_ucs4(line, -1, NULL, &written, &error);
	if  (buffer) {
		g_free(e->buffer);

		e->bufsize = e->bufused = written;
		e->curs_off = e->bufused;
		e->view_off = 0;
		e->buffer = buffer;
		lua_pushboolean(L, TRUE);
		return 1;
	} else {
		lua_pushnil(L);
		lua_pushfstring(L, "Entry:set() - UCS4 conversion failed: %s", error->message);
		g_error_free(error);
		return 2;
	}
}

static guint center_view(Entry *e, guint width) {
	/* To find a new view window, we start at the cursor, and move out, trying
	 * to keep the width used on each side roughly equal. As such, we keep a
	 * tally of the width of each side, and just advance a character on the
	 * lesser side. If we run out of characters on the left, obviously return 0
	 * and left-justify the entire line on screen. However, if we run out on the
	 * right, then pad out the right with the equivalent width of remaining chars
	 * on the left, to center properly.
	 */

	/* To simplify things, we ensure that there /is/ something on the left */
	if (e->curs_off == 0)
		return 0;

	/* To be precise, right_i starts at the cursor position itself */
	guint left_w = 0, right_w = 0;
	guint left_i = e->curs_off - 1;
	guint right_i = e->curs_off;

	while (left_w + right_w < width) {
		const guint total_width = left_w + right_w;
		/* XXX: This next if may break on terminals with less than 6 chars
		 * for the input line, I think. */
		if (left_i == 0)
			return 0;

		if (left_w < right_w || right_i == e->bufused) {
			gunichar ch = e->buffer[left_i];
			int cwidth = unicode_charwidth(ch);
			if (cwidth + total_width > width)
				break;
			left_i--;
			left_w += cwidth;
			if (right_i == e->bufused)
				right_w += cwidth;
		} else {
			gunichar ch = e->buffer[right_i];
			int cwidth = unicode_charwidth(ch);
			if (cwidth + total_width > width)
				break;
			right_i++;
			right_w += cwidth;
		}
	}
	return left_i;
}

static int try_render(Entry *e, guint lmargin) {
	guint idx = e->view_off;
	guint width = 0;
	int curs_pos = -1;
	const guint max_width = TERM_COLS - lmargin;

	term_goto(TERM_LINES - 1, lmargin);
	term_erase_eol();

	if (e->view_off >= e->bufused && e->view_off != 0)
		return -1;

	while (idx < e->bufused) {
		if (idx == e->curs_off)
			curs_pos = width + lmargin;
		gunichar ch = e->buffer[idx];
		guint charwidth = unicode_charwidth(ch);
		if (charwidth + width > max_width)
			break;
		width += charwidth;
		term_write_gunichar(ch);
		idx++;
	}

	if (idx == e->bufused && e->curs_off == e->bufused && width < max_width)
		curs_pos = width + lmargin;

	if (curs_pos != -1)
		term_goto(TERM_LINES - 1, curs_pos);
	return curs_pos;
}

static int Entry_render(LuaState *L)
{
	Entry *e      = moon_checkclass(L, "Entry", 1);
	g_assert(e->curs_off <= e->bufused);

	/* FIXME: This assumes 1 byte == 1 char */
	guint lmargin = strlen(e->prompt);
	term_goto(TERM_LINES - 1, 0);
	term_write_chars(e->prompt);

	if (try_render(e, lmargin) == -1) {
		e->view_off = center_view(e, TERM_COLS - lmargin);
		if (try_render(e, lmargin) == -1) {
			/* This should never happen, but just in case. */
			if (e->curs_off == 0) {
				e->view_off = 0;
			} else {
				e->view_off = e->curs_off - 1;
			}
			try_render(e, lmargin);
		}
	}
	return 0;
}


static void erase_region(Entry *e, int start, int end)
{
	g_assert(start <= end);
	g_assert(start <= e->bufused);
	g_assert(end   <= e->bufused);
	g_assert(start >= 0);
	g_assert(end   >= 0);

	if (start == end)
		return;
	e->dirty = 1;
	memmove(e->buffer + start, e->buffer + end,
			sizeof(e->buffer[0]) * (e->bufused - end));
	e->bufused -= (end - start);

	if (e->curs_off > end)
		e->curs_off -= (end - start);
	else if (e->curs_off > start) /* in-between */
		e->curs_off = start;
	return;
}

static int Entry_erase(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);
	int count = luaL_checkinteger(L, 2);
	if (!count)
		return 0;

	if (count > 0) {
		/* Delete chars after the current cursor location. */
		int start = e->curs_off;
		int end   = MIN(e->bufused, e->curs_off + count);
		erase_region(e, start, end);
	} else {
		/* Delete chars before the current cursor location */
		int start = e->curs_off + count;
		int end   = e->curs_off;

		/* Note: if count > e->curs_off, start will overflow and be > e->bufused */
		if (start > e->bufused)
			start = 0;
		erase_region(e, start, end);
	}
	return 0;
}

static int Entry_gc(LuaState *L)
{
	Entry *e = moon_toclass(L, "Entry", 1);
	g_free(e->buffer); /* XXX: is glib's free safe with NULL? */
	return 0;
}

static int Entry_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Entry", 1));
  	lua_pushfstring(L, "Entry (%s)", buff);
  	return 1;
}

static inline int wordlen_dir(Entry *e, int direction) {
	int count = 0;
	int i = e->curs_off;
	gboolean expect_space = 1;
	g_assert(direction);
	
	if (direction == -1)
		i--;

	for (; i >= 0 && i < e->bufused; i += direction) {
		if (expect_space) {
			expect_space = g_unichar_isspace(e->buffer[i]);
		} else {
			if (g_unichar_isspace(e->buffer[i]))
				break;
		}
		count++;
	}
	return count;
}

static int Entry_wordlen(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);

	lua_pushinteger(L, wordlen_dir(e, -1));
	lua_pushinteger(L, wordlen_dir(e, 1));
	return 2;
}

static int Entry_clear_dirty(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);

	e->dirty = 0;
	return 0;
}

static int Entry_is_dirty(LuaState *L)
{
	Entry *e  = moon_checkclass(L, "Entry", 1);
	
	lua_pushboolean(L, e->dirty);
	return 1;
}

static const LuaLReg Entry_methods[] = {
	{"new", Entry_new},
	{"keypress", Entry_keypress},
	{"set_prompt", Entry_set_prompt},
	{"move", Entry_move},
	{"move_to", Entry_move_to},
	{"get", Entry_get},
	{"set", Entry_set},
	{"clear", Entry_clear},
	{"erase", Entry_erase},
	{"render", Entry_render},
	{"wordlen", Entry_wordlen},
	{"clear_dirty", Entry_clear_dirty},
	{"is_dirty", Entry_is_dirty},
	{0, 0}
};

static const LuaLReg Entry_meta[] = {
	{"__gc", Entry_gc},
	{"__tostring", Entry_tostring},
	{0, 0}
};

int luaopen_entry(LuaState *L)
{
	moon_class_register(L, "Entry", Entry_methods, Entry_meta);
	return 1;
}
