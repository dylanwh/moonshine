#include "net.h"
#include "async.h"
#include "line-reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void on_readline(GString *line, gpointer data)
{
	char *msg = "hello, world!\n";

	g_print("Line: [%s]\n", line->str);
	async_write(GPOINTER_TO_INT(data), msg, strlen(msg));
}

static void on_error(GError *err, gpointer data)
{
	g_warning("Error: %s\n", err->message);
}

static void on_connect(int fd, gpointer data)
{
	line_reader_watch(fd, on_readline, on_error, NULL, GINT_TO_POINTER(fd));
}

static gboolean on_timeout(gpointer data)
{
	net_start();
	net_connect("localhost", "7575", on_connect, on_error, NULL);
	net_connect("localhorst", "7575", on_connect, on_error, NULL);
	return FALSE;
}

int main(int argc, char *argv[])
{
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	g_thread_init(NULL);
	g_timeout_add(2, on_timeout, loop);
	g_print("start\n");
	g_main_loop_run(loop);
}
