#ifndef BUFFER_H
#define BUFFER_H 1

#include <glib.h>

typedef struct buffer buffer;

buffer *buffer_new(guint history_len);

void buffer_set_history_len(buffer *b, guint newlen);
int buffer_get_history_len(const buffer *b);

void buffer_render(buffer *buffer);
void buffer_print(buffer *buffer, const GString *text);
void buffer_scroll(buffer *buffer, int offset);
void buffer_scroll_to(buffer *buffer, guint abs_offset);
void buffer_free(buffer *buffer);

#endif

