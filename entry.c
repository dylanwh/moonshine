/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */

#include "entry.h"
#include <slang.h>
#include <assert.h>
#include <string.h>

struct Entry {
	gunichar *buffer;
	size_t bufsize; /* The total size of the buffer in gunichars. Is zero iff buffer is NULL. */
	size_t bufused; /* The length of the actual string in the buffer. */
	size_t view_off; /* The index of the first visible character. May be out of range; entry_render will correct such issues. */
	size_t curs_off; /* The index of the character the cursor is on. Must not be out of range (> bufused). */
};

/* XXX: This should be in a header or shared .c file somewhere, buffer.c uses
 * a reimplementation too */
__attribute__((pure)) static int charwidth(gunichar ch) {
	if (g_unichar_iswide(ch))
		return 2;
	else
		return 1;
}

Entry *entry_new(lua_State *L)
{
	Entry *e = g_new(Entry, 1);
	e->buffer = NULL;
	e->bufsize = e->bufused = 0;
	e->view_off = e->curs_off = 0;
	return e;
}

void entry_free(Entry *e)
{
	if (!e) return;

	g_free(e->buffer); /* XXX: is glib's free safe with NULL? */
	g_free(e);
}

void entry_key(Entry *e, gunichar uc) {
	assert(e);
	if (!uc || !g_unichar_isdefined(uc))
		return; /* Filter invalid characters, hopefully. XXX: is this enough to deny the PUA? */
	if (e->bufused + 1 > e->bufsize) {
		/* We assume most lines will be relatively small, so just increase by 128 each time */
		e->bufsize += 128;
		e->buffer = g_renew(gunichar, e->buffer, e->bufsize);
	}
	memmove(e->buffer + e->curs_off + 1, e->buffer + e->curs_off,
            sizeof(e->buffer[0]) * e->bufused - e->curs_off);
	e->buffer[e->curs_off] = uc;
	e->bufused++;
	e->curs_off++;
}

void entry_move(Entry *e, int offset) {
	assert(e);
	if (offset == 0) return;
	
	size_t new_off = e->curs_off + offset;
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
}

void entry_move_to(Entry *e, int absolute) {
	assert(e);
	if (!e->bufused) return;

	if (absolute >= 0) {
		e->curs_off = MIN(e->bufused, absolute);
	} else { /* -1 is after the last char, -2 on the last, etc... */
		/* Note: If absolute is too far negative, we'll get size_t wraparound here. */
		e->curs_off = e->bufused + absolute + 1;
		if (e->curs_off > e->bufused)
			e->curs_off = 0;
	}
}

gchar *entry_get(Entry *e) {
	assert(e);
	if (e->bufused == 0) {
		/* buffer may be NULL, so create the result manually */
		gchar *p = g_malloc(1);
		*p = '\0';
		return p;
	}
	return g_ucs4_to_utf8(e->buffer, e->bufused, NULL, NULL, NULL);
}

void entry_clear(Entry *e) {
	assert(e);
	e->bufused = 0;
	e->curs_off = e->view_off = 0;
	/* If we have more than a page of buffer, free it, to prevent a single
	 * zomg-large string from using memory forever.
	 */
	if (e->bufused > 1024) {
		g_free(e->buffer);
		e->bufused = 0;
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
			int cwidth = charwidth(ch);
			if (cwidth + total_width > width)
				break;
			left_i--;
			left_w += cwidth;
			if (right_i == e->bufused)
				right_w += cwidth;
		} else {
			gunichar ch = e->buffer[right_i];
			int cwidth = charwidth(ch);
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
	const guint max_width = SLtt_Screen_Cols - lmargin;

	SLsmg_gotorc(SLtt_Screen_Rows - 1, lmargin);
	SLsmg_erase_eol();

	if (e->view_off >= e->bufused && e->view_off != 0)
		return -1;

	while (idx < e->bufused) {
		if (idx == e->curs_off)
			curs_pos = width + lmargin;
		gunichar ch = e->buffer[idx];
		guint charwidth;
		if (g_unichar_iswide(ch))
			charwidth = 2;
		else
			charwidth = 1;
		if (charwidth + width > max_width)
			break;
		width += charwidth;
		SLsmg_write_char(ch);
		idx++;
	}
	g_debug("try_render; width=%d maxwidth=%d; idx=%d; bu=%d; bs=%d; co=%d; vo=%d; cp=%d\n",
			width, 
			max_width, 
			idx, 
			(guint) e->bufused,
			(guint) e->bufsize,
			(guint) e->curs_off,
			(guint) e->view_off,
			(guint) curs_pos);

	if (e->curs_off == e->bufused && width < max_width)
		curs_pos = width;

	if (curs_pos != -1)
		SLsmg_gotorc(SLtt_Screen_Rows - 1, curs_pos);
	return curs_pos;
}

void entry_render(Entry *e, guint lmargin) {
	assert(e);
	assert(e->curs_off <= e->bufused);

	if (try_render(e, lmargin) == -1) {
		e->view_off = center_view(e, SLtt_Screen_Cols - lmargin);
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
}

void entry_erase(Entry *e, int count) {
	assert(e);
	if (!count)
		return;

	if (count > 0) {
		/* Delete chars after the current cursor location. */
		int start = e->curs_off;
		int end   = MAX(e->bufused, e->curs_off + count);
		entry_erase_region(e, start, end);
	} else {
		/* Delete chars before the current cursor location */
		int start = e->curs_off + count;
		int end   = e->curs_off;

		/* Note: if count > e->curs_off, start will overflow and be > e->bufused */
		if (start > e->bufused)
			start = 0;
		entry_erase_region(e, start, end);
	}
}

void entry_erase_region(Entry *e, int start, int end) {
	assert(e);
	assert(start <= end);
	assert(start <= e->bufused);
	assert(end   <= e->bufused);
	assert(start >= 0);
	assert(end   >= 0);

	if (start == end)
		return;
	memmove(e->buffer + start, e->buffer + end, e->bufused - end);
	e->bufused -= (end - start);

	if (e->curs_off > end)
		e->curs_off -= (end - start);
	else if (e->curs_off > start) /* in-between */
		e->curs_off = start;
}
