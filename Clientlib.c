#include "moonshine.h"

#define CLIENT "Client"

typedef struct {
	LuaState *L;
	GConn *conn;
	int callbacks;
} Client;

static Client *toClient (LuaState *L, int index)
{
  	Client *c = lua_touserdata(L, index);
  	if (c == NULL) luaL_typerror(L, index, CLIENT);
  	return *c;
}

static Client *checkClient (LuaState *L, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	Client *c = luaL_checkudata(L, index, CLIENT);
  	if (c == NULL) luaL_typerror(L, index, CLIENT);
  	return c;
}

static Client *pushClient (LuaState *L, const char *hostname, int port, int callbacks)
{
  	Client *c = lua_newuserdata(L, sizeof(Client));
  	luaL_getmetatable(L, CLIENT);
  	lua_setmetatable(L, -2);
  	c->L         = L;
  	c->callbacks = callbacks;
  	c->conn      = gnet_conn_new(hostname, port, on_event, c);
  	return c;
}

static int Client_new (LuaState *L)
{
	const char *hostname = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	lua_pushvalue(L, 3);
	int callbacks = luaL_ref(L, LUA_REGISTRYINDEX);

  	pushClient(L, hostname, port, callbacks);
  	return 1;
}

static int Client_keypress(LuaState *L)
{
  	Client *c = checkClient(L, 1);
  	const char *c = luaL_checkstring(L, 2);
  	gunichar uc = g_utf8_get_char(c);
  	entry_keypress(c, uc);
  	return 0;
}

static int Client_move(LuaState *L)
{
	Client *c = checkClient(L, 1);
	int offset = luaL_checkinteger(L, 2);
	entry_move(c, offset);
	return 0;
}

static int Client_move_to(LuaState *L)
{
	Client *c     = checkClient(L, 1);
	int absolute = luaL_checkinteger(L, 2);
	entry_move_to(c, absolute);
	return 0;
}

static int Client_get(LuaState *L)
{
	Client *c = checkClient(L, 1);
	char *s = entry_get(c);
	lua_pushstring(L, s);
	g_free(s);
	return 1;
}

static int Client_clear(LuaState *L)
{
	Client *c = checkClient(L, 1);
	entry_clear(c);
	return 0;
}

static int Client_erase(LuaState *L)
{
	Client *c = checkClient(L, 1);
	int count = luaL_checkinteger(L, 2);
	entry_erase(c, count);
	return 0;
}


static int Client_render(LuaState *L)
{
	Client *c = checkClient(L, 1);
	guint lmargin = luaL_optint(L, 2, 0);
	entry_render(c, lmargin);
	return 0;
}

static const LuaLReg Client_methods[] = {
  	{"new",           Client_new},
  	{"keypress",      Client_keypress},
  	{"move",          Client_move},
  	{"move_to",       Client_move_to},
  	{"get",           Client_get},
  	{"clear",         Client_clear},
  	{"erase",         Client_erase},
  	{"render",        Client_render},
  	{0, 0}
};

static int Client_gc (LuaState *L)
{
	Client *c = toClient(L, 1);
	entry_free(c);
  	return 0;
}

static int Client_tostring (LuaState *L)
{
  	char buff[32];
  	sprintf(buff, "%p", toClient(L, 1));
  	lua_pushfstring(L, "Client (%s)", buff);
  	return 1;
}

static const LuaLReg Client_meta[] = {
  	{"__gc",       Client_gc},
  	{"__tostring", Client_tostring},
  	{"__call",     Client_call},
  	{0, 0}
};

void Clientlib_open (LuaState *L)
{
	moon_class_create(L, CLIENT, Client_methods, Client_meta);
}
