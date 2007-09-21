#include "async.h"
#include "config.h"
#include <string.h>
#include <fcntl.h>


static GHashTable *context_table = NULL;

typedef struct {
	AsyncReadlineFunc on_readline;
	AsyncErrorFunc on_error;
	AsyncCleanupFunc on_cleanup;
	gpointer data;

	GIOChannel *channel;
	GString *buffer;
	GQueue *queue;

	guint tag;
	gboolean alive;
} AsyncContext;

/* {{{ Declare Private Functions */
static void async_flush(AsyncContext *ctx);
inline static void on_input(AsyncContext *ctx);
static gboolean on_event(GIOChannel *ch, GIOCondition cond, gpointer data);
static AsyncContext *async_context(int fd);

/* }}} */

void async_watch(int fd,
		AsyncReadlineFunc on_readline,
		AsyncErrorFunc on_error,
		AsyncCleanupFunc on_cleanup,
		gpointer data)
{
	g_assert(context_table);
	g_return_if_fail(fd >= 0);

	AsyncContext *ctx = g_new(AsyncContext, 1);

	ctx->buffer = g_string_new("");
	ctx->queue  = g_queue_new();
	ctx->channel = g_io_channel_unix_new(fd);
	ctx->on_readline = on_readline;
	ctx->on_error    = on_error;
	ctx->on_cleanup  = on_cleanup;
	ctx->data        = data;
	ctx->alive       = TRUE;

	g_io_channel_set_encoding(ctx->channel, NULL, NULL);
	g_io_channel_set_buffered(ctx->channel, FALSE);
	g_io_channel_set_flags(ctx->channel, G_IO_FLAG_NONBLOCK, NULL);
	ctx->tag = g_io_add_watch(ctx->channel, G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_event, ctx);

	g_hash_table_insert(context_table, GINT_TO_POINTER(fd), ctx);
}

void async_write(int fd, const char *str)
{
	AsyncContext *ctx = async_context(fd);
	g_queue_push_tail(ctx->queue, g_string_new(str));
}

void each_g_string(GString *msg, UNUSED gpointer data)
{
	g_string_free(msg, TRUE);
}

void async_close(int fd)
{
	AsyncContext *ctx = async_context(fd);
	g_hash_table_remove(context_table, GINT_TO_POINTER(fd));

	g_string_free(ctx->buffer, TRUE);
	g_queue_foreach(ctx->queue, (GFunc)each_g_string, NULL);
	g_source_remove(ctx->tag);
	g_io_channel_shutdown(ctx->channel, TRUE, NULL);
	g_io_channel_unref(ctx->channel);
	if (ctx->on_cleanup)
		ctx->on_cleanup(ctx->data);
	g_free(ctx);
}

static void async_flush( AsyncContext *ctx )/*{{{*/
{
	g_return_if_fail(ctx != NULL);

	if (ctx->buffer->len > 0) {
		ctx->on_readline(ctx->buffer, ctx->data);
		g_string_truncate(ctx->buffer, 0);
	}
}/*}}}*/

static AsyncContext *async_context(int fd)/*{{{*/
{
	g_assert(context_table);
	g_return_val_if_fail(fd >= 0, NULL);
	AsyncContext *ctx = g_hash_table_lookup(context_table, GINT_TO_POINTER(fd));
	if (ctx == NULL) {
		g_warning("No context for fd=%d\n", fd);
	}
	g_return_val_if_fail(ctx != NULL, NULL);
	return ctx;
}/*}}}*/

inline static void on_input(AsyncContext *ctx)/*{{{*/
{
	GError *err = NULL;
	gchar str[256];
	gsize len = 0;
	GIOStatus status;
	
	memset(str, 0, sizeof(str));
	status = g_io_channel_read_chars(ctx->channel, str, sizeof(str), &len, &err);

	g_assert(err == NULL);

	if (len > 0) {
		GString *buffer = ctx->buffer;
		g_string_append_len(buffer, str, len);

		/* FIXME: optimize this. */
		for (int i = 0; i < buffer->len; i++) {
			if (buffer->str[i] == '\n') {
				buffer->str[i] = '\0';
				GString *line = g_string_new(buffer->str);
				g_string_erase(buffer, 0, i+1);
				ctx->on_readline(line, ctx->data);
				g_string_free(line, TRUE);
				i = 0;
			}
		}
	}

	switch (status) {
		case G_IO_STATUS_NORMAL:
			ctx->alive = TRUE;
			break;
		case G_IO_STATUS_AGAIN:
			ctx->alive = TRUE;
			break;
		case G_IO_STATUS_EOF:
			{
				GError *err = g_error_new(ASYNC_ERROR, ASYNC_ERROR_EOF, "Got EOF");
				ctx->on_error(err, ctx->data);
				g_error_free(err);
				async_flush(ctx);
				ctx->alive = FALSE;
			}
			break;
		case G_IO_STATUS_ERROR:
			{
				GError *err = g_error_new(ASYNC_ERROR, ASYNC_ERROR_WTF, "WTF?");
				ctx->on_error(err, ctx->data);
				g_error_free(err);
				async_flush(ctx);
				ctx->alive = FALSE;

			}
			break;
	}
}/*}}}*/

inline static void on_output(AsyncContext *ctx)/*{{{*/
{
	GQueue *queue = ctx->queue;
	GError *err = NULL;
	if (g_queue_is_empty(queue)) return;

	GString *msg = (GString *)g_queue_pop_head(queue);
	gsize len = 0;
	GIOStatus status = g_io_channel_write_chars(ctx->channel, msg->str, msg->len, &len, &err);

	g_assert(err == NULL);
	g_assert(status == G_IO_STATUS_NORMAL);

	if (len < msg->len) {
		g_string_erase(msg, 0, len);
		g_queue_push_head(queue, msg);
	} else if (len == msg->len) {
		g_string_free(msg, TRUE);
	} else {
		g_assert_not_reached();
	}
}/*}}}*/

static gboolean on_event(GIOChannel *ch, GIOCondition cond, gpointer data)/*{{{*/
{
	AsyncContext *ctx = data;
	ctx->alive = TRUE;

	g_assert((cond & G_IO_NVAL) == 0);
	g_assert((cond & G_IO_ERR)  == 0);

	if (cond & G_IO_HUP) {
		GError *err = g_error_new(ASYNC_ERROR, ASYNC_ERROR_HUP, "Received HUP");
		ctx->on_error(err, ctx->data);
		g_error_free(err);
		ctx->alive = FALSE;
	}
	
	if (cond & G_IO_IN)
		on_input(ctx);

	if (cond & G_IO_OUT)
		on_output(ctx);

	if (!ctx->alive) {
		g_print("Died!\n");
	}
	return ctx->alive;
}/*}}}*/

/* {{{ Magic */
__attribute__((constructor)) static void async_init(void)
{
	context_table = g_hash_table_new(NULL, NULL);
}

__attribute__((destructor)) static void async_reset(void)
{
	g_hash_table_destroy(context_table);
}
/* }}} */
