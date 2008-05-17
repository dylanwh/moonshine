/* vim: set ts=4 sw=4 noexpandtab cindent: */
#include "moon.h"
#include "config.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/** This GError domain represents errors from getaddrinfo() */
#define NET_ERROR_DNS g_quark_from_string("NetErrorDNS")

/** This GError domain represents errors from socket or connect. The code field is
 * errno value. */
#define NET_ERROR_SYS g_quark_from_string("NetErrorSys")

/* {{{ Globals */
static GThreadPool *net_pool; /// Receives NetRequests (from net_connect()), sends NetResponse.
static GAsyncQueue *net_queue; /// Queue which holds NetResponse structures.
static GSource     *net_source; /// Connects net_queue to the GMainLoop.
static gboolean    net_is_initialized = FALSE;
/* }}} */

/* {{{ Structures and typedefs */
typedef struct addrinfo AddrInfo;

typedef enum {
	NET_CONNECT,
	NET_LISTEN,
} NetRequestType;

typedef struct {
	LuaState *L;
	NetRequestType type;
	char *hostname;
	char *service;
	int callback;
} NetRequest;

typedef enum {
	NET_FD,
	NET_ERROR,
} NetResponseType;

typedef struct {
	NetRequest *req;
	NetResponseType type;
	union {
		int fd;
		GError *error;
	} variant;
} NetResponse;

/* }}} */

/* {{{ Worker functions */
static NetResponse *net_pool_connect(NetRequest *req, AddrInfo *result)
{
	NetResponse *resp = g_new(NetResponse, 1);
	AddrInfo *rp = NULL;
	int my_errno = 0;
	int fd = -1;

	resp->req = req;
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
		resp->type    = NET_FD;
		resp->variant.fd = fd;
	} else {
		resp->type       = NET_ERROR;
		resp->variant.error = g_error_new_literal(NET_ERROR_SYS, my_errno, g_strerror(my_errno));
	}

	return resp;
}

static NetResponse *net_pool_listen(NetRequest *req, AddrInfo *result)
{
	NetResponse *resp = g_new(NetResponse, 1);
	AddrInfo *rp = NULL;
	int my_errno = 0;
	int fd = -1;

	resp->req = req;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, 
				rp->ai_protocol);
        if (fd == -1)
            continue;
        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0 && listen(fd, 5) == 0)
        	break; /* Success */
        my_errno = errno;
        close(fd);
    }

	if (rp && fd != -1) {
		resp->type    = NET_FD;
		resp->variant.fd = fd;
	} else {
		resp->type       = NET_ERROR;
		resp->variant.error = g_error_new_literal(NET_ERROR_SYS, my_errno, g_strerror(my_errno));
	}

	return resp;
}

static void net_pool_worker(NetRequest *req, gpointer data)/*{{{*/
{
	g_assert(req);
	g_assert(data == NULL);

	AddrInfo dns_hints = {
		.ai_family   = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP,
		.ai_flags    = req->type == NET_LISTEN ? AI_PASSIVE : 0,
	};
	NetResponse *resp = NULL;	
	AddrInfo *result;
	errno = 0;
	int error = getaddrinfo(req->hostname, req->service, &dns_hints, &result);
	
	if (error == 0) {
		switch (req->type) {
			case NET_CONNECT:
				resp = net_pool_connect(req, result);
				break;
			case NET_LISTEN:
				resp = net_pool_listen(req, result);
				break;
		}
	} else {
		/* FIXME: Some say gai_strerror is not thread safe. */
		resp->type       = NET_ERROR;
		resp->variant.error = g_error_new_literal(NET_ERROR_DNS, error, gai_strerror(error));
	}

	if (result && error == 0)
		freeaddrinfo(result);

	g_async_queue_push(net_queue, resp);
	g_main_context_wakeup(NULL);
}/*}}}*/

static gboolean net_source_worker(NetResponse *resp)/*{{{*/
{
	g_assert(resp);
	LuaState *L = resp->req->L;
	int callback = resp->req->callback;
	int nargs = 0;

	moon_pushref(L, callback);
	switch (resp->type) {
		case NET_FD:
		{
			int fd = resp->variant.fd;
			nargs = 1;
			lua_pushinteger(L, fd);
			break;
		}
		case NET_ERROR:
		{
			GError *err = resp->variant.error;
			nargs = 2;
			lua_pushnil(L);
			lua_pushstring(L, err->message);
			g_error_free(err);
			break;
		}
		default: g_assert_not_reached();
	}
    if (lua_pcall(L, nargs, 0, 0) != 0) {
    	g_warning("error running net.%s function with %d args: %s",
    			resp->req->type == NET_CONNECT ? "connect" : "listen",
    			nargs,
    			lua_tostring(L, -1));
    	lua_pop(L, 1);
    }

	g_free(resp->req->hostname);
	g_free(resp->req->service);
	moon_unref(L, callback);
	g_free(resp->req);
	g_free(resp);
 	return TRUE;
}/*}}}*/

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

static int net_connect(LuaState *L)/*{{{*/
{
	g_assert(net_pool && net_queue && net_source);
	const char *hostname = luaL_checkstring(L, 1);
	const char *service  = lua_tostring(L, 2);
	int callback         = moon_ref(L, 3);

	NetRequest *req = g_new(NetRequest, 1);// freed in net_source_worker.
	req->type       = NET_CONNECT;
	req->hostname   = g_strdup(hostname);  // freed in net_source_worker.
	req->service    = g_strdup(service);   // freed in net_source_worker.
	req->L          = L;
	req->callback   = callback;

	g_thread_pool_push(net_pool, req, NULL);
	return 0;
}/*}}}*/

static int net_listen(LuaState *L)/*{{{*/
{
	g_assert(net_pool && net_queue && net_source);
	const char *hostname = luaL_optstring(L, 1, NULL);
	const char *service  = lua_tostring(L, 2);
	int callback         = moon_ref(L, 3);

	NetRequest *req = g_new(NetRequest, 1);// freed in net_source_worker.
	req->type       = NET_LISTEN;
	if (req->hostname)
		req->hostname   = g_strdup(hostname);  // freed in net_source_worker.

	req->service    = g_strdup(service);   // freed in net_source_worker.
	req->L          = L;
	req->callback   = callback;

	g_thread_pool_push(net_pool, req, NULL);
	return 0;
}/*}}}*/

static LuaLReg functions[] = {
	{ "connect", net_connect },
	{ "listen", net_listen },
	{0, 0},
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

/* }}} */ 
