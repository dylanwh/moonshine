#ifndef __MOONSHINE_ASYNC_H__
#define __MOONSHINE_ASYNC_H__
#include <glib.h>

#define ASYNC_ERROR g_quark_from_string("AsyncError")

typedef enum {
	ASYNC_ERROR_HUP,
	ASYNC_ERROR_EOF,
	ASYNC_ERROR_WTF
} AsyncError;

typedef void (*AsyncReadFunc)(char *str, gsize len, gpointer data);
typedef void (*AsyncErrorFunc)(GError *err, gpointer data);
typedef void (*AsyncCloseFunc)(gpointer data);

void async_watch(int fd,
		AsyncReadFunc on_read,
		AsyncErrorFunc on_error,
		AsyncCloseFunc on_close,
		gpointer data);

void async_write(int fd, const char *str, gsize bytes);
void async_close(int fd);

void async_init(void);
void async_reset(void);

#endif
