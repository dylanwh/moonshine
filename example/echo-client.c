#include "net.h"
#include "async.h"
#include <stdio.h>

static void on_readline_stdin(GString *line, gpointer data)
{
	int fd = GPOINTER_TO_INT(data);
	async_write(fd, line->str);
	async_write(fd, "\n");
}

static void on_readline(GString *line, gpointer data)
{
	async_write(fileno(stdout), line->str);
	async_write(fileno(stdout), "\n");
}

static void on_error(GError *err, gpointer data)
{
	g_print("Error: %s\n", err->message);
}

static void on_connect(int fd, gpointer data)
{
	async_watch(fileno(stdin), on_readline_stdin, on_error, NULL, GINT_TO_POINTER(fd));
	async_watch(fileno(stdout), NULL, on_error, NULL, GINT_TO_POINTER(fd));
	async_watch(fd, on_readline, on_error, NULL, NULL);
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
