/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80: */

#include <glib.h>
#include <slang.h>
#include <stdlib.h>
#include <string.h>

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

	/** Counters for list purging. history_max is the maximum amount of
	 * scrollback to keep; scrollback the number of elements between head and
	 * view; scrollfwd the number of elements between view and tail.
	 */
	guint history_max, scrollback, scrollfwd;
};

static void purge(Buffer *b) {
	if (b->scrollback > b->history_max) {
		GList *head = b->head;
		guint reap  = b->scrollback - b->history_max;
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

Buffer *buffer_new(guint history_max) {
	g_assert(history_max > 0);
	Buffer *b = g_new(Buffer, 1);
	b->head = b->view = b->tail = NULL;
	b->history_max = history_max;
	b->scrollback = b->scrollfwd = 0;
	return b;
}

void buffer_set_history_max(Buffer *b, guint newmax) {
	g_assert(newmax > 0);
	b->history_max = newmax;
	purge(b);
}

int buffer_get_history_max(const Buffer *b) {
	return b->history_max;
}

static guint line_render(const char *line, guint bottom_row, guint top_row) {
	/* XXX: this doesn't yet handle wrapping */
	g_assert(bottom_row >= top_row);
	SLsmg_gotorc(bottom_row, 0);
	SLsmg_write_nstring((char *)line, SLtt_Screen_Cols);
	return bottom_row - 1;
}

void buffer_render(Buffer *buffer) {
	int top_row = 1;
	int bottom_row = SLtt_Screen_Rows - 2;
	char blanks[SLtt_Screen_Cols + 1];
	GList *ptr = buffer->view;

	blanks[SLtt_Screen_Cols] = '\0';
	memset(blanks, ' ', SLtt_Screen_Cols);


	for (int i = top_row; i <= bottom_row; i++) {
		SLsmg_gotorc(i, 0);
		SLsmg_write_nstring(blanks, SLtt_Screen_Cols);
	}

	while (ptr && bottom_row >= top_row) {
		bottom_row = line_render(ptr->data, bottom_row, top_row);
		ptr = ptr->prev;
	}
}

void buffer_print(Buffer *buffer, const GString *text) {
	char *copy = g_strdup(text->str);
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
