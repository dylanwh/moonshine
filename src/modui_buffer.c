/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */
#include <moonshine/config.h>
#include <moonshine/term.h>
#include <moonshine/lua.h>

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#define CLASS "moonshine.ui.buffer"

/* {{{ Buffer structure */
typedef struct bufferline {
	struct bufferline *prev, *next;
	unsigned int group;
	char text[0];
} bufferline_t;

typedef struct {
	/* These are three pointers into a doubly-linked list of lines (as strings,
	 * for now).  head and tail, of course, point to the head and tail of the
	 * list. view is the newest line visible on screen (which is != tail iff
	 * we're scrolled up).
	 *
	 * Note that all three pointers are NULL for an empty b.
	 */
	bufferline_t *head; ///< head of the list.
	bufferline_t *view; ///< view is the newest line visible on screen (which is != tail iff we're scrolled up).
	bufferline_t *tail; ///< tail of the list.

	/** Counters for list purging. histsize is the maximum amount of
	 * scrollback to keep; scrollback the number of elements between head and
	 * view; scrollfwd the number of elements between view and tail.
	 */
	guint histsize, scrollback, scrollfwd, groupcount, curgroup;
	gboolean is_dirty;
} Buffer;
/* }}} */

/* {{{ Utility functions */
static void purge(Buffer *b) {/*{{{*/
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
	ms_term_goto(bottom_row, 0);

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
		const guint max_width = MS_TERM_COLS; 
		guint cur_width       = thisline->margin;
		guint next_color      = color;

		gboolean in_word = !g_unichar_isspace(g_utf8_get_char(line));
		gboolean advance_line = TRUE;
		while (*seg_end) {
			gunichar ch = g_utf8_get_char(seg_end);
			if (ch == MS_TERM_INDENT_MARK_UCS) {
				margin = cur_width;
				advance_line = FALSE;
				next_line = g_utf8_next_char(seg_end);
				break;
			}
			if (ch >= MS_TERM_COLOR_MIN_UCS && ch <= MS_TERM_COLOR_MAX_UCS) {
				temp_margin = cur_width;
				next_color = ch - MS_TERM_COLOR_MIN_UCS;
				advance_line = FALSE;
				next_line = g_utf8_next_char(seg_end);
				break;
			}
			guint ch_len = ms_term_charwidth(ch);
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
		ms_term_goto(bottom_row, lines->margin);
		ms_term_color_use_id(lines->color);
		ms_term_write_chars_to((unsigned char *)lines->start, (unsigned char *)lines->end);

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
static int buffer_new(LuaState *L)/*{{{*/
{
	guint histsize = luaL_optint(L, 2, 1024);
	Buffer *b      = ms_lua_newclass(L, CLASS, sizeof(Buffer));
	b->head        = NULL;
	b->view        = NULL;
	b->tail        = NULL;
	b->histsize    = histsize;
	b->scrollback  = b->scrollfwd = 0;
	b->curgroup    = 0;
	b->is_dirty       = TRUE;
	return 1;
}/*}}}*/

static int buffer_set_histsize(LuaState *L)/*{{{*/
{
	Buffer *b = ms_lua_checkclass(L, CLASS, 1);
	guint newsize = luaL_checkinteger(L, 2);
	b->histsize = newsize;
	purge(b);
	b->is_dirty = TRUE;
	return 0;
}/*}}}*/

static int buffer_get_histsize(LuaState *L)/*{{{*/
{
	Buffer *b = ms_lua_checkclass(L, CLASS, 1);
	lua_pushinteger(L, b->histsize);
	return 1;
}/*}}}*/

static int buffer_is_dirty(LuaState *L)/*{{{*/
{
	Buffer *b = ms_lua_checkclass(L, CLASS, 1);
	if (!lua_isnone(L, 2)) {
		gboolean val = lua_toboolean(L, 2);
		b->is_dirty = val;
	}

	lua_pushboolean(L, b->is_dirty);
	return 1;
}/*}}}*/

static int buffer_render(LuaState *L)/*{{{*/
{
	Buffer *b = ms_lua_checkclass(L, CLASS, 1);
	int top_row = luaL_checkinteger(L, 2);
	int bottom_row = luaL_checkinteger(L, 3);

	bufferline_t *ptr = b->view;

	for (int i = top_row; i <= bottom_row; i++) {
		ms_term_goto(i, 0);
		ms_term_erase_eol();
	}

	while (ptr && bottom_row >= top_row) {
		bottom_row = line_render(ptr->text, bottom_row, top_row);
		ptr = ptr->prev;
	}
	b->is_dirty = FALSE;
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

static int buffer_print(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);
	const char *text = luaL_checkstring(L, 2);

	if (!g_utf8_validate(text, -1, NULL)) {
		return luaL_error(L, CLASS ":print - UTF8 validation failed.");
	}
	do_print(b, text);
	b->is_dirty = TRUE;
	lua_pushboolean(L, 1);
	return 1;
}/*}}}*/

static int buffer_scroll(LuaState *L)/*{{{*/
{
	Buffer *b  = ms_lua_checkclass(L, CLASS, 1);
	int offset = luaL_checkinteger(L, 2);

	if (offset > 0)
		scroll_up(b, offset);
	else {
		scroll_down(b, -offset);
		purge(b);
	}
	b->is_dirty = TRUE;
	return 0;
}/*}}}*/

static int buffer_scroll_to(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);
	guint abs_offset = luaL_checkinteger(L, 2);

	/* Scroll down to the bottom by twiddling pointers, then scroll up to the
	 * desired position
	 */
	b->view = b->tail;
	b->scrollback += b->scrollfwd;
	b->scrollfwd = 0;
	scroll_up(b, abs_offset);
	purge(b);
	b->is_dirty = TRUE;
	return 0;
}/*}}}*/

static int buffer_get_current(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);

	if (b->view) {
		lua_pushstring(L, b->view->text);
		return 1;
	} else {
		lua_pushnil(L);
		return 1;
	}
}/*}}}*/

static int buffer_at_end(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);

	lua_pushboolean(L, b->tail == b->view);
	return 1;
}/*}}}*/

static int buffer_set_group_id(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);
	int gid			 = luaL_checkinteger(L, 2);

	b->curgroup = gid;
	return 0;
}/*}}}*/

static int buffer_clear_group_id(LuaState *L)/*{{{*/
{
	Buffer *b        = ms_lua_checkclass(L, CLASS, 1);
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

static int buffer_reprint(LuaState *L)/*{{{*/
{
	Buffer *b			= ms_lua_checkclass(L, CLASS, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	int group			= luaL_checkinteger(L, 3);
	int max				= luaL_checkinteger(L, 4);

	int count = 0;

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
	for (; l && l != oldtail->next; l = l->next) {
		if (l->group != group) continue;

		lua_pushvalue(L, 2);
		lua_pushstring(L, l->text);
		lua_call(L, 1, 1);
		gboolean matched = lua_toboolean(L, -1);
		lua_pop(L, 1);

		if (matched) {
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
						g_assert(delp != b->view);
						g_free(delp);
						if (b->view)
							b->scrollfwd--;
						else
							b->scrollback--;
						delp = nextp;
					}
					b->tail = oldtail;
					if (!b->view)
						b->view = b->tail;
					g_assert(b->tail); /* an empty result is not possible; we must've reprinted SOMETHING */
					b->tail->next = NULL;
				} else {
					do_print(b, l->text);
				}
			}
		}
	}
	b->histsize = old_histsize;
	purge(b);

	lua_pushinteger(L, count);
	b->is_dirty = TRUE;
	return 1;
}/*}}}*/

static int buffer_clear_lines(LuaState *L)/*{{{*/
{
	Buffer *b		= ms_lua_checkclass(L, CLASS, 1);
	int count		= luaL_checkinteger(L, 2);
	int realcount   = 0;

	for (; b->tail && realcount < count; ) {
		bufferline_t *prev = b->tail->prev;
		if (b->view == b->tail) {
			b->view = prev;
			b->scrollback--;
		} else {
			b->scrollfwd--;
		}
		if (prev)
			prev->next = NULL;
		else
			b->head = NULL;
		g_free(b->tail);
		b->tail = prev;
		realcount++;
	}
	if (!b->tail) {
		g_assert(!b->view);
		b->head = NULL;
	}
	lua_pushinteger(L, realcount);
	b->is_dirty = TRUE;
	return 1;
}/*}}}*/
/* }}} */

/* {{{ Meta methods */
static int buffer_gc(LuaState *L)/*{{{*/
{
	Buffer *b = ms_lua_toclass(L, CLASS, 1);
	for(bufferline_t *ptr = b->head; ptr;) {
		bufferline_t *next = ptr->next;
		g_free(ptr);
		ptr = next;
	}
	return 0;
}/*}}}*/

static int buffer_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
  	sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
  	lua_pushfstring(L, "Buffer (%s)", buff);
  	return 1;
}/*}}}*/
/* }}} */

static const LuaLReg buffer_methods[] = {/*{{{*/
	{"new", buffer_new},
	{"set_histsize", buffer_set_histsize},
	{"get_histsize", buffer_get_histsize},
	{"get_current", buffer_get_current},
	{"render", buffer_render},
	{"print", buffer_print},
	{"scroll", buffer_scroll},
	{"scroll_to", buffer_scroll_to},
	{"at_end", buffer_at_end},
	{"set_group_id", buffer_set_group_id},
	{"clear_group_id", buffer_clear_group_id},
	{"reprint", buffer_reprint},
	{"clear_lines", buffer_clear_lines},
	{"is_dirty", buffer_is_dirty},
	{0, 0}
};/*}}}*/

static const LuaLReg buffer_meta[] = {/*{{{*/
	{"__gc", buffer_gc},
	{"__tostring", buffer_tostring},
	{0, 0}
};
/*}}}*/

int luaopen_moonshine_ui_buffer(LuaState *L)/*{{{*/
{
	ms_lua_class_register(L, CLASS, buffer_methods, buffer_meta);
	return 1;
}/*}}}*/
