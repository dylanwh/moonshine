/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */
#include "term.h"
#include "config.h"
#include "moon.h"

#include <glib.h>
#include <slang.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* {{{ Buffer structure */
typedef struct {
	/* These are three pointers into a doubly-linked list of lines (as strings,
	 * for now).  head and tail, of course, point to the head and tail of the
	 * list. view is the newest line visible on screen (which is != tail iff
	 * we're scrolled up).
	 *
	 * Note that all three pointers are NULL for an empty b.
	 */
	GList *head; ///< head of the list.
	GList *view; ///< tail of the list.
	GList *tail; ///< view is the newest line visible on screen (which is != tail iff we're scrolled up).

	/** Counters for list purging. histsize is the maximum amount of
	 * scrollback to keep; scrollback the number of elements between head and
	 * view; scrollfwd the number of elements between view and tail.
	 */
	guint histsize, scrollback, scrollfwd;
} Buffer;
/* }}} */

/* {{{ Utility functions */
static void purge(Buffer *b) {/*{{{*/
	if (b->scrollback > b->histsize) {
		GList *head = b->head;
		guint reap  = b->scrollback - b->histsize;
		g_assert(head);
		/* Walk from the tail down to tail + reap, freeing strings as we go.
		 * Then just break the link between the two lists, and free the dead
		 * list in one step.
		 */
		GList *ptr  = head;
		for (int i = 0; i < reap; i++) {
			g_assert(ptr);
			g_free(ptr->data);
			ptr = ptr->next;
		}
		/* ptr now points to the last element we want to /keep/ */   
		ptr->prev->next = NULL;
		ptr->prev       = NULL;
		g_list_free(head);
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
		const guint max_width = TERM_COLS; //SLtt_Screen_Cols;
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
			guint ch_len;
			if (g_unichar_iswide(ch))
				ch_len = 2;
			else
				ch_len = 1;
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
		SLsmg_set_color(lines->color);
		SLsmg_write_chars((unsigned char *)lines->start, (unsigned char *)lines->end);

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
	guint histsize = luaL_optint(L, 1, 1024);
	g_return_val_if_fail(histsize > 0, 0);
	Buffer *b = moon_newclass(L, "Buffer", sizeof(Buffer));
	b->head = b->view = b->tail = NULL;
	b->histsize = histsize;
	b->scrollback = b->scrollfwd = 0;
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

	int top_row = 1;
	int bottom_row = SLtt_Screen_Rows - 2;
	GList *ptr = b->view;

	for (int i = top_row; i <= bottom_row; i++) {
		term_goto(i, 0);
		term_erase_eol();
	}

	while (ptr && bottom_row >= top_row) {
		bottom_row = line_render(ptr->data, bottom_row, top_row);
		ptr = ptr->prev;
	}
	return 0;
}/*}}}*/

static int Buffer_print(LuaState *L)/*{{{*/
{
	Buffer *b        = moon_checkclass(L, "Buffer", 1);
	const char *text = luaL_checkstring(L, 2);
	g_assert(g_utf8_validate(text, -1, NULL));

	char *copy = g_strdup(text);
	GList *elem = g_list_alloc();
	elem->data = copy;
	
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
	return 0;
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

static int Buffer_format(LuaState *L)/*{{{*/
{
	const char *input = luaL_checkstring(L, 1);
	GString *out = g_string_sized_new(strlen(input));
	const gchar *p = input;

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
/* }}} */

/* {{{ Meta methods */
static int Buffer_gc(LuaState *L)/*{{{*/
{
	Buffer *b = moon_toclass(L, "Buffer", 1);
	for(GList *ptr = b->head; ptr; ptr = ptr->next)
		g_free(ptr->data);
	g_list_free(b->head);
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
	{"render", Buffer_render},
	{"print", Buffer_print},
	{"scroll", Buffer_scroll},
	{"scroll_to", Buffer_scroll_to},
	{"format", Buffer_format},
	{"format_escape", Buffer_format_escape},
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
