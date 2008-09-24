#include <moonshine/config.h>
#include <moonshine/ms-lua.h>
#include <gnet.h>

#define CLASS "moonshine.net.client"
#define REFS  "moonshine.net._client"

/* {{{ Client Structure */
typedef struct {
	GConn *conn;
	MSLuaRef *ref;
} Client; /* }}} */

/* {{{ Utility functions */
inline static int push_event(LuaState *L, GConnEvent *event)/*{{{*/
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
}/*}}}*/

// push a table on to the stack where the values
// are weak.
inline static void push_weaktable(LuaState *L)/*{{{*/
{
	lua_newtable(L); // push table.
	lua_newtable(L); // push metatable.	
	lua_pushstring(L, "v");
	lua_setfield(L, -1, "__mode");
	lua_setmetatable(L, -2);
}/*}}}*/

static void client_callback(GConn *conn, GConnEvent *event, gpointer userdata)/*{{{*/
{
	MSLuaRef *func = userdata;
	LuaState *L    = ms_lua_pushref(func);    // push function
	int argc       = 1;
	lua_getfield(L, LUA_REGISTRYINDEX, REFS); // push REFS
	lua_pushlightuserdata(L, conn);           // push light ud
	lua_gettable(L, -2);                      // pop light ud, push value of REFS[ud]
	g_assert(!lua_isnil(L, -1));    
	lua_remove(L, -2);                        // pop REFS
	argc += push_event(L, event);      // push 1 or 2 arguments onto the stack.

	// call func(client, event_type [, buffer ]). 
	if (lua_pcall(L, argc, 0, 0)) 
		g_warning("moonshine error in client callback: %s", lua_tostring(L, -1));
}/*}}}*/
/* }}} */

/* {{{ Methods */
static int client_new(LuaState *L)/*{{{*/
{
	const char *host = luaL_checkstring(L, 2);
	guint port       = luaL_checkinteger(L, 3);
	MSLuaRef *func   = ms_lua_ref(L, 4);
	GConn  *conn     = gnet_conn_new(host, port, client_callback, func);
	Client *client   = ms_lua_newclass(L, CLASS, sizeof(Client)); 

	client->conn = conn;
	client->ref  = func;

	lua_getfield(L, LUA_REGISTRYINDEX, REFS); // push REFS
	lua_pushlightuserdata(L, conn);           // push conn
	lua_pushvalue(L, -3);                     // push copy of client.
	lua_settable(L, -3);                      // REFS[conn] = client
	lua_pop(L, 1);

	return 1;
}/*}}}*/

static int client_connect(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_connect(client->conn);

	return 0;
}/*}}}*/

static int client_disconnect(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_disconnect(client->conn);

	return 0;
}/*}}}*/

static int client_is_connected(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	lua_pushboolean(L, gnet_conn_is_connected(client->conn));

	return 1;
}/*}}}*/

static int client_read(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gnet_conn_read(client->conn);

	return 0;
}/*}}}*/

static int client_readn(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	gint length    = luaL_checkinteger(L, 2);

	gnet_conn_readn(client->conn, length);

	return 0;
}/*}}}*/

static int client_write(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_checkclass(L, CLASS, 1);
	size_t length = 0;
	const char *buffer = luaL_checklstring(L, 1, &length);

	gnet_conn_write(client->conn, (char *)buffer, length);

	return 0;
}/*}}}*/
/* }}} */

/* {{{ Meta Methods */
static int client_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
  	sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
  	lua_pushfstring(L, "Client (%s)", buff);
  	return 1;
}/*}}}*/

static int client_gc(LuaState *L)/*{{{*/
{
	Client *client = ms_lua_toclass(L, CLASS, 1);
	gnet_conn_delete(client->conn);
	ms_lua_unref(client->ref);

	return 0;
}/*}}}*/
/* }}} */

static const LuaLReg client_methods[] = {/*{{{*/
	{"new", client_new},
	{"connect", client_connect},
	{"disconnect", client_disconnect},
	{"is_connected", client_is_connected},
	{"read", client_read},
	{"readn", client_readn},
	{"write", client_write},
	{0, 0}
};/*}}}*/

static const LuaLReg client_meta[] = {/*{{{*/
	{"__gc", client_gc},
	{"__tostring", client_tostring},
	{0, 0}
};/*}}}*/

int luaopen_moonshine_net_client(LuaState *L)/*{{{*/
{
	push_weaktable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, REFS);
	ms_lua_class_register(L, CLASS, client_methods, client_meta);

	return 1;
}/*}}}*/
