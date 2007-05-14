#ifndef BUFFER_H
#define BUFFER_H 1

#include <glib.h>

typedef struct Buffer Buffer;

Buffer *buffer_new(guint history_len);

void buffer_set_history_len(Buffer *b, guint newlen);
int buffer_get_history_len(const Buffer *b);

void buffer_render(Buffer *buffer);
void buffer_print(Buffer *buffer, const GString *text);
void buffer_scroll(Buffer *buffer, int offset);
void buffer_scroll_to(Buffer *buffer, guint abs_offset);
void buffer_free(Buffer *buffer);

#endif

