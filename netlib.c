#include "moonshine.h"
#define NET "net"

static GHashTable *servers;

typedef struct {
	GConn *conn; // Only in the context to support reclaiming memory.
	char *tag;
	LuaState *L;
} Context;

static void callback(GConn *conn, GConnEvent *event, gpointer data)
{
	Context *ctx = data;
	switch (event->type)
	{
	  	case GNET_CONN_CONNECT:
	  	    {
	  	      	gnet_conn_timeout (conn, 0);	/* reset timeout */
	  	      	gnet_conn_readline (conn);
	  	      	moon_call(ctx->L, "on_connect", "s", ctx->tag);
	  	      	break;
	  	    }
	  	case GNET_CONN_READ:
	  	    {
	  	      	/* Write line out */
	  	      	moon_call(ctx->L, "on_read", "ss", ctx->tag, event->buffer);
	  	      	gnet_conn_readline (conn);
	  	      	break;
	  	    }
	  	case GNET_CONN_WRITE:
	  	    {
	  	      	/* do nothing */
	  	      	break;
	  	    }

	  	case GNET_CONN_CLOSE:
	  	    {
	  	      	//gnet_conn_delete (conn);
	  	      	moon_call(ctx->L, "on_close", "s", ctx->tag);
	  	      	break;
	  	    }

	  	case GNET_CONN_TIMEOUT:
	  	    {
	  	      	//gnet_conn_delete (conn);
	  	      	moon_call(ctx->L, "on_timeout", "s", ctx->tag);
	  	      	break;
	  	    }

	  	case GNET_CONN_ERROR:
	  	    {
	  	      	//gnet_conn_delete (conn);
	  	      	moon_call(ctx->L, "on_error", "s", ctx->tag);
	  	      	break;
	  	    }
	  	default: g_assert_not_reached ();
	}
}

/* connect(tag, host, port) */
static int net_connect(LuaState *L)
{
	g_assert(servers);
	const char *tag  = luaL_checkstring(L, 1);
	const char *host = luaL_checkstring(L, 2);
	int port         = luaL_checkint(L, 3);

	Context *ctx = g_hash_table_lookup(servers, tag);
	if (!ctx) {
		ctx = g_new(Context, 1);
		GConn *conn = gnet_conn_new(host, port, callback, ctx);
		ctx->tag    = g_strdup(tag);
		ctx->L      = L;
		ctx->conn   = conn;
		g_hash_table_insert(servers, ctx->tag, ctx);
	}
	g_assert(ctx);
	g_assert(ctx->conn);
	gnet_conn_connect(ctx->conn);
	gnet_conn_set_watch_error (ctx->conn, TRUE);
	gnet_conn_timeout (ctx->conn, 30000);
	return 0;
}

/* disconnect(tag) */
static int net_disconnect(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = g_hash_table_lookup(servers, tag);

	if (ctx)
		gnet_conn_disconnect(ctx->conn);
	return 0;
}

/* write(tag, msg) */
static int net_write(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	const char *msg  = luaL_checkstring(L, 2);

	Context *ctx = g_hash_table_lookup(servers, tag);
	if (ctx)
		gnet_conn_write(ctx->conn, msg, strlen(msg));
	return 0;
}

static LuaLReg functions[] = {
	{ "connect", net_connect, },
	{ "disconnect",  net_disconnect },
	{ "write", net_write },
	{ 0, 0 }
};

void netlib_open(LuaState *L)
{
	luaL_register(L, NET, functions);
	lua_pop(L, 1);

	servers = g_hash_table_new(NULL, NULL);
}

