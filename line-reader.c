#include "line-reader.h"

LineReader *line_reader_new(
		LineReaderFunc on_line,
		AsyncErrorFunc on_error,
		AsyncCloseFunc on_close, 
		gpointer data)
{
	LineReader *lr = g_new(LineReader, 1);
	lr->buffer   = g_string_new("");
	lr->on_line  = on_line;
	lr->on_error = on_error;
	lr->on_close = on_close;
	lr->data = data;

	return lr;
}

void line_reader_on_read(gchar *str, gsize len, gpointer data)
{
	LineReader *lr = data;
	GString *buffer = lr->buffer;
	g_string_append_len(buffer, str, len);
	for (int i = 0; i < buffer->len; i++) {
		if (buffer->str[i] == '\n') {
			GString *line = g_string_new_len(buffer->str, i);
			g_string_erase(buffer, 0, i+1);
			i = 0;
			lr->on_line(line, lr->data);
			g_string_free(line, TRUE);
		}
	}
}

void line_reader_on_error(GError *err, gpointer data)
{
	LineReader *lr = data;
	lr->on_error(err, lr->data);
}

void line_reader_on_close(gpointer data)
{
	LineReader *lr = data;
	if (lr->on_close)
		lr->on_close(lr->data);
	g_string_free(lr->buffer, TRUE);
	g_free(lr);
}


