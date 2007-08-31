#include "moonshine.h"
#define NET "net"

static GHashTable *servers;

typedef struct {
	GConn *conn; // Only in the context to support reclaiming memory.
	int callback;
	char *tag;
	LuaState *L;
} Context;

static Context *context_get(LuaState *L, const char *tag)
{
	g_assert(servers);
	g_assert(tag);
	Context *ctx = g_hash_table_lookup(servers, tag);
	if (!ctx)
		luaL_error(L, "Unknown net.tag: %s", tag);
	return ctx;
}

PURE static const char *event_name(GConnEventType type)
{
	switch (type) {
	  	case GNET_CONN_CONNECT: return "connect";
	  	case GNET_CONN_READ: return "read";
	  	case GNET_CONN_WRITE: return "write";
	  	case GNET_CONN_CLOSE: return "close";
	  	case GNET_CONN_TIMEOUT: return "timeout";
	  	case GNET_CONN_ERROR: return "error";
	  	default: g_assert_not_reached ();
	}
}

static void on_event(GConn *conn, GConnEvent *event, gpointer data)
{
	const char *name = event_name(event->type);
	Context *ctx = data;
	lua_rawgeti(ctx->L, LUA_REGISTRYINDEX, ctx->callback);
	lua_pushstring(ctx->L, ctx->tag);
	lua_pushstring(ctx->L, name);
	int nargs = 2;

	switch (event->type) {
		case GNET_CONN_CONNECT:
			gnet_conn_timeout (conn, 0);	/* reset timeout */
			break;
		case GNET_CONN_READ:
			lua_pushstring(ctx->L, event->buffer);
			nargs++;
			break;
		default:
			break;
	}

    if (lua_pcall(ctx->L, nargs, 0, 0) != 0)
    	g_error("error running function for net tag `%s': %s",
    			ctx->tag, lua_tostring(ctx->L, -1));
}

static int net_create(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	const char *host = luaL_checkstring(L, 2);
	int port         = luaL_checkint(L, 3);
	lua_pushvalue(L, 4);
	int callback     = luaL_ref(L, LUA_REGISTRYINDEX);

	if (g_hash_table_lookup(servers, tag))
		luaL_error(L, "net.tag %s already created", tag);
	Context *ctx = g_new0(Context, 1);
	GConn *conn = gnet_conn_new(host, port, on_event, ctx);
	
	ctx->callback = callback;
	ctx->tag      = g_strdup(tag);
	ctx->L        = L;
	ctx->conn     = conn;

	g_hash_table_insert(servers, ctx->tag, ctx);
	return 0;
}

static int net_destroy(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;

	g_hash_table_remove(servers, ctx->tag);
	luaL_unref(L, LUA_REGISTRYINDEX, ctx->callback);
	gnet_conn_delete(ctx->conn);
	g_free(ctx->tag);
	g_free(ctx);
	return 0;
}

static int net_connect(LuaState *L)
{	
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;
	gnet_conn_connect(ctx->conn);
	gnet_conn_set_watch_error (ctx->conn, TRUE);
	gnet_conn_timeout (ctx->conn, 30000);
	return 0;
}

static int net_disconnect(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;
	gnet_conn_disconnect(ctx->conn);
	return 0;
}

static int net_write(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	const char *msg  = luaL_checkstring(L, 2);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;
	gnet_conn_write(ctx->conn, (char *)msg, strlen(msg));
	return 0;
}

static int net_readline(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;
	gnet_conn_readline(ctx->conn);
	return 0;
}

static int net_hostname(LuaState *L)
{
	const char *tag  = luaL_checkstring(L, 1);
	Context *ctx = context_get(L, tag);
	if (!ctx) return 0;
	lua_pushstring(L, ctx->conn->hostname);
	return 1;
}


static LuaLReg functions[] = {
	{ "create", net_create, },
	{ "destroy", net_destroy, },
	{ "connect", net_connect, },
	{ "disconnect",  net_disconnect },
	{ "write", net_write },
	{ "readline", net_readline },
	{ "hostname", net_hostname },
	{ 0, 0 }
};

void netlib_open(LuaState *L)
{
	luaL_register(L, NET, functions);
	lua_pop(L, 1);
	servers = g_hash_table_new(g_str_hash, g_str_equal);
}
