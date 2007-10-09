#include "moon.h"
#include "net.h"

typedef struct Context {
	LuaState *L;
	int connect;
	int error;
} Context;


static void cleanup(Context *ctx)
{
	moon_unref(ctx->L, ctx->connect);
	moon_unref(ctx->L, ctx->error);
	g_free(ctx);
}

static void on_connect(int fd, gpointer data)
{
	Context *ctx = data;
	moon_deref(ctx->L, ctx->connect);
	lua_pushinteger(ctx->L, fd);
	
	if (lua_pcall(ctx->L, 1, 0, 0) != 0)
		g_warning("Error calling net.connect connection callback: %s",
				lua_tostring(ctx->L, -1));

	cleanup(ctx);
}

static void on_error(GError *error, gpointer data)
{
	Context *ctx = data;
	moon_deref(ctx->L, ctx->error);
	lua_pushstring(ctx->L, g_quark_to_string(error->domain));
	lua_pushinteger(ctx->L, error->code);
	lua_pushstring(ctx->L, error->message);
	if (lua_pcall(ctx->L, 3, 0, 0) != 0)
		g_warning("Error calling net.connect error callback: %s",
				lua_tostring(ctx->L, -1));

	cleanup(ctx);
}

static int net_wrap_connect(LuaState *L)
{
	const char *hostname = luaL_checkstring(L, 1);
	const char *service  = luaL_checkstring(L, 2);
	int connect_cb       = moon_ref(L, 3);
	int error_cb         = moon_ref(L, 4);

	Context *ctx = g_new(Context, 1);
	ctx->L       = L;
	ctx->connect = connect_cb;
	ctx->error   = error_cb;
	
	net_connect(hostname, service, on_connect, on_error, ctx);
	return 0;
}

static LuaLReg functions[] = {
	{ "connect", net_wrap_connect }
};

int luaopen_net(LuaState *L)
{
	net_init();
	luaL_register(L, "net", functions);
	return 1;
}
