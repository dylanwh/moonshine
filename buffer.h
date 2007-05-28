/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80: */
#ifndef __MOONSHINE_BUFFER_H__
#define __MOONSHINE_BUFFER_H__
#include <glib.h>

/** \file buffer.h
 * \brief buffer class. */

typedef struct Buffer Buffer;

/** \brief Create a new buffer.
 *
 * \param history_len The maximum size of the scrollback buffer
 */
Buffer *buffer_new(guint history_len);

/** \brief Change the scrollback buffer size of an existing buffer.
 * 
 *  The buffer will be truncated immediately if need be.
 *  
 * \param b The buffer to modify
 * \param newlen The new maximum scrollback length
 */
void buffer_set_history_len(Buffer *b, guint newlen);

/** \brief Returns the scrollback length limit of a buffer.
 *
 * \param b The buffer to query
 */
int buffer_get_history_len(const Buffer *b);

/** \brief Renders a buffer to the screen
 *
 * Currently, this renders to the region of the window starting at the second
 * line down, and ending at the third-to-last line from the bottom.
 *
 * FIXME: take a region argument
 *
 * \param buffer The buffer to render
 */
void buffer_render(Buffer *buffer);

/** \brief Appends a string to a buffer
 *
 * The buffer scrollback will be truncated as needed. If text is not valid
 * UTF-8, the behavior is undefined.
 *
 * \param buffer The buffer to print to
 * \param text A null-terminated string of UTF-8 text to render. This string
 *   will be copied into space managed by the buffer.
 */
void buffer_print(Buffer *buffer, const char *text);

/** \brief Scrolls the buffer
 *
 * Scrolls the buffer up by (offset) lines. If offset is negative, then the
 * buffer will scroll down instead. If the attempted scroll would scroll off
 * the limits of the buffer, it will instead scroll as far as it can.
 *
 * Note that this function does not take line wrapping into account; the offset
 * is measured in terms of logical lines.
 *
 * \param buffer The buffer to scroll
 * \param offset The number of logical lines to scroll up
 */
void buffer_scroll(Buffer *buffer, int offset);

/** \brief Scrolls the buffer to an absolute position
 *
 * Scrolls the buffer to a position (abs_offset) above the bottom of the
 * screen. Equivalent to:
 * <code>
 *   buffer_scroll(buffer, INT_MIN);
 *   buffer_scroll(buffer, offset);
 * </code>
 * except that the scrolling operation is atomic, and does not truncate the
 * buffer until it reaches the final position.
 *
 * \param buffer The buffer to scroll
 * \param buffer The offset to scroll to
 */
void buffer_scroll_to(Buffer *buffer, guint abs_offset);

/** \brief Destroys a buffer and all associated storage
 *
 * \param buffer The buffer to destroy
 */
void buffer_free(Buffer *buffer);

#define INDENT_MARK_UCS 0xF0000
#define INDENT_MARK_UTF "\xF3\xB0\x80\x80"

#endif

