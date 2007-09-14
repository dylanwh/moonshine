/* vim: set ft=c.doxygen: */
#include "net.h"
#include "config.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* {{{ Globals */
static GThreadPool *net_pool; /// Receives NetRequests (from net_connect()), sends NetResponses.
static GAsyncQueue *net_queue; /// Queue which holds NetResponse structures.
static GSource     *net_source; /// Connects net_queue to the GMainLoop.
/* }}} */

/* {{{ Structures and typedefs */
typedef struct addrinfo AddrInfo;
typedef struct {
	char *hostname; /// copy of the hostname passed to net_connect()
	char *service; /// copy of service passed to net_connect()
	NetFunc func; /// the function to call upon either error or sucessful function.
	gpointer data; /// environemnt of above function.
} NetRequest;

typedef struct {
	NetEvent event;
	NetFunc func;
	gpointer data;
} NetResponse;
/* }}} */

/* {{{ Worker functions */
static void net_pool_worker(NetRequest *req, gpointer data)
{
	g_assert(data == NULL);
	g_assert(req);
	static AddrInfo dns_hints = {
		.ai_family   = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP,
	};
	NetResponse *resp = g_new(NetResponse, 1); // freed in net_source_worker().
	resp->func = req->func;
	resp->data = req->data;

	AddrInfo *result;
	errno = 0;
	int error = getaddrinfo(req->hostname, req->service, &dns_hints, &result);

	if (error == 0) {
		AddrInfo *rp = NULL;
		int fd = -1;
		for (rp = result; rp != NULL; rp = rp->ai_next) {
			fd = socket(rp->ai_family, rp->ai_socktype, 
					rp->ai_protocol);
        	if (fd == -1)
            	continue;
        	if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
        		break; /* Success */
        	close(fd);
        }

		if (rp && fd != -1) {
			resp->event.type = NET_CONNECT;
			resp->event.fd   = fd;
		} else {
			resp->event.type  = NET_SYS_ERROR;
			resp->event.error = 0; // no addrinfo error
			resp->event.sys_error = errno;
		}
	} else {
		resp->event.type  = NET_ERROR;
		resp->event.error = error;
		resp->event.sys_error = errno;
	}

	g_free(req->hostname);
	g_free(req->service);
	g_free(req);
	freeaddrinfo(result);

	g_async_queue_push(net_queue, resp);
	g_main_context_wakeup(NULL);
}

static gboolean net_source_worker(NetResponse *resp)
{
	g_assert(resp);
	g_assert(resp->func);
	resp->func(resp->event, resp->data);
	g_free(resp);
 	return TRUE;
}
/* }}} */

/* {{{ GSource boilerplate */
static gboolean net_source_prepare(GSource *src, gint *timeout)//{{{
{
	g_assert(net_queue);
	*timeout = -1;
	return g_async_queue_length(net_queue) > 0;
}//}}}
static gboolean net_source_check(GSource *src) { //{{{

	g_assert(net_queue);
	return g_async_queue_length(net_queue) > 0;
}//}}}
static gboolean net_source_dispatch (//{{{
		UNUSED GSource *src,
		GSourceFunc func, 
		UNUSED gpointer user_data)
{
	gpointer data = g_async_queue_pop(net_queue);
	gboolean result = func(data);
	return result;
}//}}}
static GSourceFuncs net_source_functions = {//{{{
	net_source_prepare,
	net_source_check, 
	net_source_dispatch, 
	NULL
};//}}}
/* }}} */

/* {{{ Public functions */

void net_start(void)
{
	net_queue  = g_async_queue_new();
	net_source = g_source_new(&net_source_functions, sizeof(GSource));
	g_source_set_callback(net_source, (GSourceFunc) net_source_worker, NULL, NULL);

	net_pool   = g_thread_pool_new((GFunc) net_pool_worker, NULL, -1, FALSE, NULL);

	g_source_attach(net_source, NULL);
}

void net_connect(const char *hostname, const char *service, NetFunc func, gpointer data)
{
	g_assert(net_pool && net_queue && net_source);
	g_return_if_fail(hostname);
	g_return_if_fail(service);

	NetRequest *req = g_new(NetRequest, 1);// freed in net_pool_worker.
	req->hostname = g_strdup(hostname);    // freed in net_pool_worker.
	req->service  = g_strdup(service);     // freed in net_pool_worker.
	req->data     = data;
	req->func     = func;
	g_thread_pool_push(net_pool, req, NULL);
}

void net_stop(void)
{
	// FIXME
}

/* }}} */
