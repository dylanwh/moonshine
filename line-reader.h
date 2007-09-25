#ifndef __MOONSHINE_LINEREADER_H__
#define __MOONSHINE_LINEREADER_H__
#include <glib.h>
#include "async.h"

typedef void (*LineReaderFunc)(GString *line, gpointer data);

typedef struct {
	GString *buffer;
	LineReaderFunc on_line;
	AsyncErrorFunc on_error;
	AsyncCloseFunc on_close;
	gpointer data;
} LineReader;

LineReader *line_reader_new(
		LineReaderFunc on_line,
		AsyncErrorFunc on_error,
		AsyncCloseFunc on_close, 
		gpointer data);

void line_reader_on_read(gchar *str, gsize len, gpointer data);
void line_reader_on_error(GError *err, gpointer data);
void line_reader_on_close(gpointer data);

#define line_reader_watch(fd, on_line, on_error, on_close, data) \
	async_watch(fd, \
			line_reader_on_read, \
			line_reader_on_error, \
			line_reader_on_close, \
			line_reader_new(on_line, on_error, on_close, data))


#endif
