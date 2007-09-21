#ifndef __MOONSHINE_ASYNC_H__
#define __MOONSHINE_ASYNC_H__

#include <glib.h>

#define ASYNC_ERROR g_quark_from_string("AsyncError")

typedef enum {
	ASYNC_ERROR_HUP,
	ASYNC_ERROR_EOF,
	ASYNC_ERROR_WTF
} AsyncError;

typedef void (*AsyncReadlineFunc)(GString *line, gpointer data);
typedef void (*AsyncErrorFunc)(GError *err, gpointer data);
typedef void (*AsyncCleanupFunc)(gpointer data);

void async_watch(int fd,
		AsyncReadlineFunc on_readline,
		AsyncErrorFunc on_error,
		AsyncCleanupFunc on_cleanup,
		gpointer data);

void async_write(int fd, const char *str);
void async_close(int fd);


#endif
