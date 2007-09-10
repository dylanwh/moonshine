/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80: */

#include <glib.h>
#include <slang.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "moonshine.h"
#include "buffer.h"

struct Buffer {
	/* These are three pointers into a doubly-linked list of lines (as strings,
	 * for now).  head and tail, of course, point to the head and tail of the
	 * list. view is the newest line visible on screen (which is != tail iff
	 * we're scrolled up).
	 *
	 * Note that all three pointers are NULL for an empty buffer.
	 */
	GList *head; ///< head of the list.
	GList *view; ///< tail of the list.
	GList *tail; ///< view is the newest line visible on screen (which is != tail iff we're scrolled up).

	/** Counters for list purging. histsize is the maximum amount of
	 * scrollback to keep; scrollback the number of elements between head and
	 * view; scrollfwd the number of elements between view and tail.
	 */
	guint histsize, scrollback, scrollfwd;
};

static void purge(Buffer *b) {
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
}

Buffer *buffer_new(guint histsize) {
	g_assert(histsize > 0);
	Buffer *b = g_new(Buffer, 1);
	b->head = b->view = b->tail = NULL;
	b->histsize = histsize;
	b->scrollback = b->scrollfwd = 0;
	return b;
}

void buffer_set_histsize(Buffer *b, guint newmax) {
	g_assert(newmax > 0);
	b->histsize = newmax;
	purge(b);
}

guint buffer_get_histsize(const Buffer *b) {
	return b->histsize;
}

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
				/* This word extends beyond the current buffer.
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

void buffer_render(Buffer *buffer) {
	int top_row = 1;
	int bottom_row = SLtt_Screen_Rows - 2;
	GList *ptr = buffer->view;

	for (int i = top_row; i <= bottom_row; i++) {
		term_goto(i, 0);
		term_erase_eol();
	}

	while (ptr && bottom_row >= top_row) {
		bottom_row = line_render(ptr->data, bottom_row, top_row);
		ptr = ptr->prev;
	}
}

void buffer_print(Buffer *buffer, const char *text) {
	assert(g_utf8_validate(text, -1, NULL));

	char *copy = g_strdup(text);
	GList *elem = g_list_alloc();
	elem->data = copy;
	
	elem->prev = buffer->tail;
	if (buffer->tail)
		buffer->tail->next = elem;
	if (!buffer->head)
		buffer->head = elem;
	if (buffer->view == buffer->tail) {
		buffer->view = elem;
		buffer->scrollback++;
	} else {
		buffer->scrollfwd++;
	}
	buffer->tail = elem;
	purge(buffer);
}

static void scroll_up(Buffer *buffer, guint offset) {
	while (offset-- && buffer->scrollback > 1) {
		buffer->scrollback--;
		buffer->scrollfwd++;
		g_assert(buffer->view);
		buffer->view = buffer->view->prev;
		g_assert(buffer->view);
	}
}

static void scroll_down(Buffer *buffer, guint offset) {
	while (offset-- && buffer->scrollfwd) {
		buffer->scrollback++;
		buffer->scrollfwd--;
		g_assert(buffer->view);
		buffer->view = buffer->view->next;
		g_assert(buffer->view);
	}
}

void buffer_scroll(Buffer *buffer, int offset) {
	if (offset > 0)
		scroll_up(buffer, offset);
	else {
		scroll_down(buffer, -offset);
		purge(buffer);
	}
}

void buffer_scroll_to(Buffer *buffer, guint abs_offset) {
	/* Scroll down to the bottom by twiddling pointers, then scroll up to the
	 * desired position
	 */
	buffer->view = buffer->tail;
	buffer->scrollback += buffer->scrollfwd;
	buffer->scrollfwd = 0;
	scroll_up(buffer, abs_offset);
	purge(buffer);
}

void buffer_free(Buffer *buffer) {
	for(GList *ptr = buffer->head; ptr; ptr = ptr->next)
		g_free(ptr->data);
	g_list_free(buffer->head);
	g_free(buffer);
}
