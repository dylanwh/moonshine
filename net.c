/* vim: set ts=4 sw=4 noexpandtab cindent: */
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
static gboolean    net_is_initialized = FALSE;
/* }}} */

/* {{{ Structures and typedefs */
typedef enum {
	NET_CONNECT,
	NET_ERROR,
} NetType;

typedef struct addrinfo AddrInfo;
typedef struct {
	LuaState *L;
	char *hostname;
	char *service;
	int callback;
} NetRequest;

typedef struct {
	NetRequest *req;
	NetType type;
	union {
		int fd;
		GError *error;
	} data;
} NetResponse;
/* }}} */

/* {{{ Worker functions */
static void net_pool_worker(NetRequest *req, gpointer data)
{
	g_assert(req);
	g_assert(data == NULL);

	static AddrInfo dns_hints = {
		.ai_family   = AF_UNSPEC,
		.ai_socktypeOA = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP,
	};

	NetResponse *resp = g_new(NetResponse, 1);
	resp->req = req;
	
	AddrInfo *result;
	errno = 0;
	int error = getaddrinfo(req->hostname, req->service, &dns_hints, &result);
	if (error == 0) {
		AddrInfo *rp = NULL;
		int my_errno;
		int fd = -1;
		for (rp = result; rp != NULL; rp = rp->ai_next) {
			fd = socket(rp->ai_family, rp->ai_socktype, 
					rp->ai_protocol);
        	if (fd == -1)
            	continue;
        	if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
        		break; /* Success */
        	my_errno = errno;
        	close(fd);
        }

		if (rp && fd != -1) {
			resp->type    = NET_CONNECT;
			resp->data.fd = fd;
		} else {
			resp->type       = NET_ERROR;
			resp->data.error = g_error_new_literal(NET_ERROR_SYS, my_errno, g_strerror(my_errno));
		}
	} else {
		/* FIXME: Some say gai_strerror is not thread safe. */
		resp->type       = NET_ERROR;
		resp->data.error = g_error_new_literal(NET_ERROR_DNS, error, gai_strerror(error));
	}

	if (result && error == 0)
		freeaddrinfo(result);

	g_async_queue_push(net_queue, resp);
	g_main_context_wakeup(NULL);
}

static gboolean net_source_worker(NetResponse *resp)
{
	g_assert(resp);

	switch (resp->type) {
		case NET_CONNECT:
		{

			int fd = resp->data.fd;
			moon_pushref(L, resp->req->callback);
			lua_newtable(L);
			lua_pushinteger(L, fd);
			break;
		}
		case NET_ERROR:
		{

			break;
		}
	}
	g_free(resp->req->hostname);
	g_free(resp->req->service);
	lua_unref(resp->req->callback);
	g_free(resp->req);
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

/* {{{ Lua stuff */
static LuaLReg functions[] = {
	{ "connect", net_connect }
};

int luaopen_net(LuaState *L)
{
	if (!net_is_initialized) {
		net_is_initialized = TRUE;
		net_queue  = g_async_queue_new();
		net_source = g_source_new(&net_source_functions, sizeof(GSource));
		g_source_set_callback(net_source, (GSourceFunc) net_source_worker, NULL, NULL);
		net_pool   = g_thread_pool_new((GFunc) net_pool_worker, NULL, -1, FALSE, NULL);
		g_source_attach(net_source, NULL);
	}
	luaL_register(L, "net", functions);
	return 1;
}

static int net_connect(LuaState *L)
{
	g_assert(net_pool && net_queue && net_source);
	const char *hostname = luaL_checkstring(L, 1);
	const char *service  = lua_tostring(L, 2);
	int callback         = moon_ref(L, 3);

	NetRequest *req = g_new(NetRequest, 1);// freed in net_source_worker.
	req->hostname   = g_strdup(hostname);  // freed in net_source_worker.
	req->service    = g_strdup(service);   // freed in net_source_worker.
	req->L          = L;
	req->callback   = callback;

	g_thread_pool_push(net_pool, req, NULL);
}
/* }}} */

