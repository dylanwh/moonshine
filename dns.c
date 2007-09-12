#include "dns.h"

typedef struct {
	char *hostname;
	struct addrinfo *result;
	int error;
	DNSCallback callback;
	gpointer data;
} DNSContext;

GAsyncQueue *dns_queue;
static GSource     *dns_source;
static GThreadPool *dns_pool;

static gboolean dns_source_prepare(GSource *src, gint *timeout)
{
	*timeout = -1;
	//g_print("Preparing source...\n");
	return g_async_queue_length(dns_queue) > 0;
}

static gboolean dns_source_check(GSource *src)
{
	//g_print("Checking source...\n");
	return g_async_queue_length(dns_queue) > 0;
}

static gboolean dns_source_dispatch (
		UNUSED GSource *src,
		GSourceFunc func, 
		UNUSED gpointer user_data)
{
	//g_print("Dispatching source...\n");
	gpointer data = g_async_queue_pop(dns_queue);
	gboolean result = func(data);
	return result;
}

static GSourceFuncs dns_source_functions = {
	dns_source_prepare,
	dns_source_check,
	dns_source_dispatch,
	NULL,
};


static void dns_pool_worker(DNSContext *ctx, gpointer data)
{
	//g_print("dns_pool_worker: starts\n");
	g_assert(data == NULL);
	g_assert(ctx);
	struct addrinfo *result;
	ctx->error = getaddrinfo(ctx->hostname, NULL, NULL, &result);
	ctx->result = result;
	g_async_queue_push(dns_queue, ctx);
	g_main_context_wakeup(NULL);
	//g_print("dns_pool_worker: quits\n");
}

static gboolean dns_source_worker(DNSContext *ctx)
{
	//g_print("dns_source_worker: starts\n");
	g_assert(ctx->callback);
	ctx->callback(ctx->hostname, ctx->result, ctx->error, ctx->data);
	g_free(ctx->hostname);
	if (ctx->result)
		freeaddrinfo(ctx->result);
	g_free(ctx);
	//g_print("dns_source_worker: quits\n");
 	return TRUE;
}

void dns_start(void)
{
	dns_queue  = g_async_queue_new();
	dns_source = g_source_new(&dns_source_functions, sizeof(GSource));
	g_source_set_callback(dns_source, (GSourceFunc) dns_source_worker, NULL, NULL);

	dns_pool   = g_thread_pool_new((GFunc) dns_pool_worker, NULL, -1, FALSE, NULL);

	g_source_attach(dns_source, NULL);
}

void dns_lookup(const char *hostname, DNSCallback callback, gpointer data)
{
	//g_print("dns_lookup: starts\n");
	DNSContext *ctx = g_new(DNSContext, 1);
	ctx->hostname = g_strdup(hostname);
	ctx->data     = data;
	ctx->callback = callback;
	g_thread_pool_push(dns_pool, ctx, NULL);
	//g_print("dns_lookup: quits\n");
}
