/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */
#include "term.h"
#include "config.h"
#include "moon.h"
#include "util.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct bufferline {
	struct bufferline *prev, *next;
	unsigned int group;
	char text[0];
} bufferline_t;

/* {{{ Buffer structure */
typedef struct {
	/* These are three pointers into a doubly-linked list of lines (as strings,
	 * for now).  head and tail, of course, point to the head and tail of the
	 * list. view is the newest line visible on screen (which is != tail iff
	 * we're scrolled up).
	 *
	 * Note that all three pointers are NULL for an empty b.
	 */
	bufferline_t *head; ///< head of the list.
	bufferline_t *view; ///< tail of the list.
	bufferline_t *tail; ///< view is the newest line visible on screen (which is != tail iff we're scrolled up).

	/** Counters for list purging. histsize is the maximum amount of
	 * scrollback to keep; scrollback the number of elements between head and
	 * view; scrollfwd the number of elements between view and tail.
	 */
	guint histsize, scrollback, scrollfwd, groupcount, curgroup;
} Buffer;
/* }}} */

/* {{{ Utility functions */

static void purge(Buffer *b) {
	if (b->histsize == 0)
		return;
	if (b->scrollback > b->histsize) {
		bufferline_t *head = b->head;
		guint reap  = b->scrollback - b->histsize;
		g_assert(head);
		/* Walk from the tail down to tail + reap, freeing strings as we go.
		 * Then just break the link between the two lists, and free the dead
		 * list in one step.
		 */
		bufferline_t *ptr  = head;
		for (int i = 0; i < reap; i++) {
			bufferline_t *cur = ptr;
			g_assert(ptr);
			ptr = ptr->next;
			g_free(cur);
		}
		/* ptr now points to the last element we want to /keep/ */   
		ptr->prev       = NULL;
		b->head = ptr;
		b->scrollback -= reap;
	}
}/*}}}*/

/* {{{ Utility functions for rendering */
static const char *skip_space(const char *in) {
	while (*in && g_unichar_isspace(g_utf8_get_char(in)))
		in = g_utf8_next_char(in);
	return in;
}

static guint line_render(const char *line, guint bottom_row, guint top_row) {
	typedef struct plan {
		struct plan *prev;
		guint margin;
		const char *start, *end;
		gboolean advance;
		guint color;
	} plan_t;

	plan_t *lines = NULL;

	g_assert(bottom_row >= top_row);
	term_goto(bottom_row, 0);

	guint margin = 0;
	guint color  = 0;
	guint temp_margin = 0;
	while (*line) {
		plan_t *thisline = alloca(sizeof *thisline);
		thisline->prev = lines;
		thisline->margin = temp_margin ? temp_margin : margin;
		lines = thisline;
		temp_margin = 0;

		const char *seg_start = line;
		const char *seg_end   = line;
		const char *last_word = line;
		const char *next_line = NULL;
		const guint max_width = TERM_COLS; 
		guint cur_width       = thisline->margin;
		guint next_color      = color;

		gboolean in_word = !g_unichar_isspace(g_utf8_get_char(line));
		gboolean advance_line = TRUE;
		while (*seg_end) {
			gunichar ch = g_utf8_get_char(seg_end);
			if (ch == BUFFER_INDENT_MARK_UCS) {
				margin = cur_width;
				advance_line = FALSE;
				next_line = g_utf8_next_char(seg_end);
				break;
			}
			if (ch >= BUFFER_COLOR_MIN_UCS && ch <= BUFFER_COLOR_MAX_UCS) {
				temp_margin = cur_width;
				next_color = ch - BUFFER_COLOR_MIN_UCS;
				advance_line = FALSE;
				next_line = g_utf8_next_char(seg_end);
				break;
			}
			guint ch_len = unicode_charwidth(ch);
			if (cur_width + ch_len > max_width) {
				/* This word extends beyond the current b.
				 * Try to wrap in a way which doesn't break off this word. If
				 * the word's the whole line, do a hard break.
				 */
				if (seg_start != last_word)
					seg_end = last_word;
				break;
			}
			cur_width += ch_len;
			if (!in_word && !g_unichar_isspace(ch)) {
				last_word = seg_end;
				in_word = TRUE;
			}
			if (g_unichar_isspace(ch)) {
				in_word = FALSE;
			}
			seg_end = g_utf8_next_char(seg_end);
		}
		if (!next_line)
			next_line = skip_space(seg_end);
		line = next_line;

		thisline->start		= seg_start;
		thisline->end		= seg_end;
		thisline->advance	= advance_line;
		thisline->color		= color;
		color				= next_color;

	}

	while (lines && bottom_row >= top_row) {
		term_goto(bottom_row, lines->margin);
		term_color_use_id(lines->color);
		term_write_chars_to((unsigned char *)lines->start, (unsigned char *)lines->end);

		/* We want to make sure we advance at least once. So, the last line we
		 * write, we don't change bottom_row; and then we subtract at the very
		 * end.
		 *
		 * Also, since the list is in reverse order, check the /previous/ line's
		 * struct for whether we need to advance.
		 */
		if (lines->prev && lines->prev->advance)
			bottom_row--;
		lines = lines->prev;
	}

	return bottom_row - 1;
}
/* }}} */
/* {{{ Utility functions for scrolling */
static void scroll_up(Buffer *b, guint offset) {
	while (offset-- && b->scrollback > 1) {
		b->scrollback--;
		b->scrollfwd++;
		g_assert(b->view);
		b->view = b->view->prev;
		g_assert(b->view);
	}
}

static void scroll_down(Buffer *b, guint offset) {
	while (offset-- && b->scrollfwd) {
		b->scrollback++;
		b->scrollfwd--;
		g_assert(b->view);
		b->view = b->view->next;
		g_assert(b->view);
	}
}
/* }}} */
/* }}} */

/* {{{ Methods */
static int Buffer_new(LuaState *L)/*{{{*/
{
	guint histsize = luaL_optint(L, 2, 1024);
	g_return_val_if_fail(histsize > 0, 0);
	Buffer *b = moon_newclass(L, "Buffer", sizeof(Buffer));
	b->head = b->view = b->tail = NULL;
	b->histsize = histsize;
	b->scrollback = b->scrollfwd = 0;
	b->curgroup = 0;
	return 1;
}/*}}}*/

static int Buffer_set_histsize(LuaState *L)/*{{{*/
{
	Buffer *b = moon_checkclass(L, "Buffer", 1);
	guint newsize = luaL_checkinteger(L, 2);
	b->histsize = newsize;
	purge(b);
	return 0;
}/*}}}*/

static int Buffer_get_histsize(LuaState *L)/*{{{*/
{
	Buffer *b = moon_checkclass(L, "Buffer", 1);
	lua_pushinteger(L, b->histsize);
	return 1;
}/*}}}*/

static int Buffer_render(LuaState *L)/*{{{*/
{
	Buffer *b = moon_checkclass(L, "Buffer", 1);
	int top_row = luaL_checkinteger(L, 2);
	int bottom_row = luaL_checkinteger(L, 3);

	bufferline_t *ptr = b->view;

	for (int i = top_row; i <= bottom_row; i++) {
		term_goto(i, 0);
		term_erase_eol();
	}

	while (ptr && bottom_row >= top_row) {
		bottom_row = line_render(ptr->text, bottom_row, top_row);
		ptr = ptr->prev;
	}
	return 0;
}/*}}}*/

static void do_print(Buffer *b, const char *text)/*{{{*/
{
	g_assert(g_utf8_validate(text, -1, NULL));

	bufferline_t *elem = g_malloc(sizeof(bufferline_t) + strlen(text) + 1);
	strcpy(elem->text, text);
	elem->prev = elem->next = NULL;
	elem->group = b->curgroup;
	
	elem->prev = b->tail;
	if (b->tail)
		b->tail->next = elem;
	if (!b->head)
		b->head = elem;
	if (b->view == b->tail) {
		b->view = elem;
		b->scrollback++;
	} else {
		b->scrollfwd++;
	}
	b->tail = elem;
	purge(b);
}/*}}}*/

static int Buffer_print(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);
	const char *text = luaL_checkstring(L, 2);

	if (!g_utf8_validate(text, -1, NULL)) {
		lua_pushnil(L);
		lua_pushfstring(L, "Buffer:print - UTF8 validation failed.");
		return 2;
	}
	do_print(b, text);
	lua_pushboolean(L, 1);
	return 1;
}/*}}}*/

static int Buffer_scroll(LuaState *L)/*{{{*/
{
	Buffer *b  = moon_checkclass(L, "Buffer", 1);
	int offset = luaL_checkinteger(L, 2);

	if (offset > 0)
		scroll_up(b, offset);
	else {
		scroll_down(b, -offset);
		purge(b);
	}
	return 0;
}/*}}}*/

static int Buffer_scroll_to(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);
	guint abs_offset = luaL_checkinteger(L, 2);

	/* Scroll down to the bottom by twiddling pointers, then scroll up to the
	 * desired position
	 */
	b->view = b->tail;
	b->scrollback += b->scrollfwd;
	b->scrollfwd = 0;
	scroll_up(b, abs_offset);
	purge(b);
	return 0;
}/*}}}*/

static int Buffer_get_current(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);

	if (b->view) {
		lua_pushstring(L, b->view->text);
		return 1;
	} else {
		lua_pushnil(L);
		return 1;
	}
}/*}}}*/

static int Buffer_at_end(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);

	lua_pushboolean(L, b->tail == b->view);
	return 1;
}/*}}}*/

static int Buffer_format(LuaState *L)/*{{{*/
{
	const char *input = luaL_checkstring(L, 1);
	GString *out = g_string_sized_new(strlen(input));
	const gchar *p = input;
	luaL_checktype(L, 2, LUA_TTABLE);

	while (1) {
		const gchar *oldp = p;
		gchar *nextesc = strchr(p, '%');
		if (!nextesc) {
			g_string_append(out, p);
			// XXX: Maybe use lua_pushlstring?
			lua_pushstring(L, out->str);
			g_string_free(out, TRUE);
			return 1;
		}
		g_string_append_len(out, p, nextesc - p);
		switch (*(nextesc + 1)) {
			case '%':
				g_string_append_c(out, '%');
				p = nextesc + 2;
				break;
			case '|':
				g_string_append(out, BUFFER_INDENT_MARK_UTF);
				p = nextesc + 2;
				break;
			case '1' ... '9':
				{
					lua_rawgeti(L, 2, *(nextesc + 1) - '0');
					const char *s = lua_tostring(L, -1);
					if (s != NULL)
						g_string_append(out, s);
					p = nextesc + 2;
					lua_pop(L, 1);
					break;
				}
			case '{':
				{
					gchar *start = nextesc + 2;
					gchar *end = strchr(start, '}');
					if (end) {
						gchar name[end - start + 1];
						memcpy(name, start, sizeof name - 1);
						name[sizeof name - 1] = '\0';
						g_string_append(out, term_color_to_utf8(name));
						p = end + 1;
						break;
					} else { goto unknown_esc; }
				}
			default:
unknown_esc:
				g_string_append_c(out, *nextesc);
				p = nextesc + 1;
				break;
		}
		g_assert(p > oldp);
	}
	g_assert_not_reached();

}/*}}}*/

static int Buffer_format_escape(LuaState *L)/*{{{*/
{
	const char *input = luaL_checkstring(L, 1);
	GString *out = g_string_sized_new(strlen(input));

	const gchar *p = input;
	while (1) {
		gchar *nextesc = strchr(p, '%');
		if (!nextesc) {
			g_string_append(out, p);
			lua_pushstring(L, out->str);
			g_string_free(out, TRUE);
			return 1;
		}
		g_string_append_len(out, p, nextesc - p);
		g_string_append(out, "%%");
		p = nextesc + 1;
	}
	g_assert_not_reached();
}/*}}}*/

static int Buffer_set_group_id(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);
	int gid			 = luaL_checkinteger(L, 2);

	b->curgroup = gid;
	return 0;
}/*}}}*/

static int Buffer_clear_group_id(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);
	int gid			 = luaL_checkinteger(L, 2);
	
	/* group clear operations are assumed to be uncommon, so we'll
	 * just do this the inefficient way
	 */

	int cleared = 0;
	int belowview = 1;
	for(bufferline_t *l = b->tail; l; ) {
		bufferline_t *prev = l->prev;
		if (l == b->view) {
			belowview = 0;
		}
		if (l->group == gid) {
			if (b->tail == l)
				b->tail = prev;
			if (b->view == l)
				b->view = prev;
			else if (belowview)
				b->scrollfwd--;
			else
				b->scrollback--;
			g_free(l);
			cleared++;
		}
		l = prev;
	}
	if (b->tail == NULL) {
		b->head = NULL;
		g_assert(b->view == NULL);
	}

	lua_pushinteger(L, cleared);
	return 1;
}/*}}}*/

static int Buffer_reprint_matching(LuaState *L)/*{{{*/
{
	Buffer *b			= moon_checkclass(L, "Buffer", 1);
	const char *regex	= luaL_checkstring(L, 2);
	int group			= luaL_checkinteger(L, 3);
	int max				= luaL_checkinteger(L, 4);

	int count = 0;
	GError *error;
	GRegex *re = g_regex_new(regex, G_REGEX_CASELESS, 0, &error);
	if (!re) {
		lua_pushnil(L);
		lua_pushfstring(L, "Buffer:reprint_matching - Bad regex: %s", error->message);
		g_error_free(error);
		return 2;
	}

	/* having entries purged as we go makes rolling back annoying
	 * XXX: Should lastlog etc entries count to the history limit? how to handle
	 * purging them?
	 */
	guint old_histsize = b->histsize;
	b->histsize = 0;

	/* remember the old end of the buffer so we can undo everything if
	 * we go over quota
	 */
	bufferline_t *oldtail = b->tail;

	int aborted = 0;
	/* find the tail, then work our way up */
	bufferline_t *l = b->tail;
	for (; l && l->prev; l = l->prev);
	/* now start matching */
	for (; l; l = l->next) {
		if (l->group != group) continue;

		if (g_regex_match(re, l->text, 0, NULL)) {
			count++;
			if (!aborted) {
				if (count && count > max) {
					/* oops. rollback our new entries. */
					aborted = 1;
					if (b->view == b->tail)
						b->view = NULL;
					/* free the new list entries */
					for (bufferline_t *delp = oldtail ? oldtail->next : NULL; delp;) {
						bufferline_t *nextp = delp->next;
						g_assert(delp->group == b->curgroup);
						g_free(delp);
						if (b->view)
							b->scrollfwd--;
						else
							b->scrollback--;
						delp = nextp;
					}
					if (!b->view)
						b->view = b->tail;
				} else {
					do_print(b, l->text);
				}
			}
		}
	}
	b->histsize = old_histsize;
	purge(b);

	lua_pushinteger(L, count);
	return 1;
}/*}}}*/

static int Buffer_clear_lines(LuaState *L)/*{{{*/
{
	Buffer *b		= moon_checkclass(L, "Buffer", 1);
	int count		= luaL_checkinteger(L, 2);
	int realcount   = 0;

	for (bufferline_t *l = b->tail; l && realcount < count; ) {
		bufferline_t *prev = l->prev;
		if (b->view == l) {
			b->view = prev;
			b->scrollback--;
		} else {
			b->scrollfwd--;
		}
		prev->next = NULL;
		g_free(l);
		l = prev;
	}
	if (!b->tail) {
		g_assert(!b->view);
		b->head = NULL;
	}
	lua_pushinteger(L, realcount);
	return 1;
}/*}}}*/

/* }}} */

/* {{{ Meta methods */
static int Buffer_gc(LuaState *L)/*{{{*/
{
	Buffer *b = moon_toclass(L, "Buffer", 1);
	for(bufferline_t *ptr = b->head; ptr;) {
		bufferline_t *next = ptr->next;
		g_free(ptr);
		ptr = next;
	}
	return 0;
}/*}}}*/

static int Buffer_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Buffer", 1));
  	lua_pushfstring(L, "Buffer (%s)", buff);
  	return 1;
}/*}}}*/
/* }}} */

static const LuaLReg Buffer_methods[] = {/*{{{*/
	{"new", Buffer_new},
	{"set_histsize", Buffer_set_histsize},
	{"get_histsize", Buffer_get_histsize},
	{"get_current", Buffer_get_current},
	{"render", Buffer_render},
	{"print", Buffer_print},
	{"scroll", Buffer_scroll},
	{"scroll_to", Buffer_scroll_to},
	{"at_end", Buffer_at_end},
	{"format", Buffer_format},
	{"format_escape", Buffer_format_escape},
	{"set_group_id", Buffer_set_group_id},
	{"clear_group_id", Buffer_clear_group_id},
	{"reprint_matching", Buffer_reprint_matching},
	{"clear_lines", Buffer_clear_lines},
	{0, 0}
};/*}}}*/
static const LuaLReg Buffer_meta[] = {/*{{{*/
	{"__gc", Buffer_gc},
	{"__tostring", Buffer_tostring},
	{0, 0}
};
/*}}}*/

int luaopen_buffer(LuaState *L)/*{{{*/
{
	moon_class_register(L, "Buffer", Buffer_methods, Buffer_meta);
	return 1;
}/*}}}*/
