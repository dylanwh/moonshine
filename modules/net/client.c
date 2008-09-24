#include <moonshine/config.h>
#include <moonshine/ms-lua.h>
#include <gnet.h>

#define CLASS "moonshine.net.client"
#define REFS  "moonshine.net.client.refs"

typedef GConn * Client;

inline static int client_push_event(LuaState *L, GConnEvent *event)
{
	char *type;
	switch (event->type) {
		case GNET_CONN_ERROR:    type = "error"; break;
  		case GNET_CONN_CONNECT:  type = "connect"; break;
  		case GNET_CONN_CLOSE:    type = "close"; break;
  		case GNET_CONN_TIMEOUT:  type = "timeout"; break;
  		case GNET_CONN_READ:     type = "read"; break;
  		case GNET_CONN_WRITE:    type = "write"; break;
  		case GNET_CONN_READABLE: type = "readable"; break;
  		case GNET_CONN_WRITABLE: type = "writable"; break;
  		default: g_assert_not_reached(); break;
  	}
  	lua_pushstring(L, type);
  	if (event->type == GNET_CONN_READ) {
  		lua_pushlstring(L, event->buffer, event->length);
  		return 2;
  	} else {
  		return 1;
  	}
}

static void client_callback(GConn *conn, GConnEvent *event, gpointer userdata)
{
	MSLuaRef *func = userdata;
	LuaState *L    = ms_lua_pushref(func);    // push function
	int argc       = 1;
	lua_getfield(L, LUA_REGISTRYINDEX, REFS); // push REFS
	lua_pushlightuserdata(L, conn);           // push light ud
	lua_gettable(L, -2);                      // pop light ud, push value of REFS[ud]
	g_assert(!lua_isnil(L, -1));    
	lua_remove(L, -2);                        // pop REFS
	argc += client_push_event(L, event);      // push 1 or 2 arguments onto the stack.

	// call func(client, event_type [, buffer ]). 
	if (lua_pcall(L, argc, 0, 0)) 
		g_warning("moonshine error in client callback: %s", lua_tostring(L, -1));
}

static int client_new(LuaState *L)
{
	const char *host = luaL_checkstring(L, 2);
	guint port       = luaL_checkinteger(L, 3);
	MSLuaRef *func   = ms_lua_ref(L, 4);
	Client *client   = ms_lua_newclass(L, CLASS, sizeof(Client));
	*client          = gnet_conn_new(host, port, client_callback, func);

	return 1;
}

static int client_connect(LuaState *L)
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_connect(*client);

	return 0;
}

static int client_disconnect(LuaState *L)
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_disconnect(*client);

	return 0;
}

static int client_is_connected(LuaState *L)
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	lua_pushboolean(L, gnet_conn_is_connected(*client));

	return 1;
}

static int client_read(LuaState *L)
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_read(*client);

	return 0;
}


static int client_readn(LuaState *L)
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gint length    = luaL_checkinteger(L, 2);

	gnet_conn_readn(*client, length);

	return 0;
}
