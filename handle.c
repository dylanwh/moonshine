#include "moon.h"
#include "config.h"
#include <string.h>
#include <fcntl.h>

typedef struct {
	LuaState *L;
	int callback;

	GIOChannel *channel;
	GQueue *queue;
	guint tag;
	gboolean closed;
	gboolean alive;
} Handle;

/* Events that we need to call the callback on:
 * - read string -- f(h, "read", str)
 * - error.      -- f(h, "error", error)
 * - eof.        -- f(h, "eof", nil)
 * - hup.        -- f(h, "hup", nil) */

/* {{{ Event Handlers */
inline static gboolean on_input(Handle *h)/*{{{*/
{
	LuaState *L = h->L;
	GError *err = NULL;
	gchar str[256];
	gsize len = 0;
	memset(str, '\0', sizeof(str));
	GIOStatus status = g_io_channel_read_chars(h->channel, str, sizeof(str) - 1, &len, &err);

	int nargs = 0;
	gboolean rv = TRUE;

	moon_pushref(L, h->callback);
	/*lua_getfield(L, LUA_REGISTRYINDEX, "HandleTable");
	lua_pushlightuserdata(L, h);
	lua_gettable(L, -2);
	lua_remove(L, -2); // remove "HandleTable"*/
	switch (status) {
		case G_IO_STATUS_NORMAL:
			lua_pushstring(L, "read");
			lua_pushstring(L, str);
			nargs = 2;
			break;
		case G_IO_STATUS_AGAIN:
			lua_pop(L, 2);
			nargs = -1;
			break;
		case G_IO_STATUS_EOF:
			lua_pushstring(L, "eof");
			nargs = 1;
			rv = FALSE;
			break;
		case G_IO_STATUS_ERROR:
			g_assert(err != NULL);
			lua_pushstring(L, "error");
			moon_pusherror(L, err);
			g_error_free(err);
			nargs = 2;
			rv = FALSE;
			break;
	}
	if (nargs >= 0) {
    	if (lua_pcall(L, nargs, 0, 0) != 0)
    		g_warning("error running Handle callback with %d args: %s",
    				nargs,
    				lua_tostring(L, -1));
	}
	return rv;
}/*}}}*/
inline static gboolean on_output(Handle *h)/*{{{*/
{
	LuaState *L   = h->L;
	GQueue *queue = h->queue;
	GError *err = NULL;
	GString *msg = (GString *)g_queue_pop_head(queue);
	gsize len = 0;
	GIOStatus status = g_io_channel_write_chars(h->channel, msg->str, msg->len, &len, &err);

	switch (status) {
		case G_IO_STATUS_NORMAL:
			if (len < msg->len) {
				g_string_erase(msg, 0, len);
				g_queue_push_head(queue, msg);
			} else if (len == msg->len) {
				g_string_free(msg, TRUE);
			} else {
				g_warning("This situation is insane. I just wrote more bytes than I should have. Goodbye.");
				g_assert_not_reached();
			}
			break;
		case G_IO_STATUS_AGAIN:
			break;
		case G_IO_STATUS_EOF:
			/* TODO: is it possible to get EOF while writing to a socket? */
			g_assert_not_reached();
			break;
		case G_IO_STATUS_ERROR:
			g_assert(err != NULL);
			moon_pushref(L, h->callback);
			/*lua_getfield(L, LUA_REGISTRYINDEX, "HandleTable");
			lua_pushlightuserdata(L, h);
			lua_gettable(L, -2);
			lua_remove(L, -2); // remove "HandleTable"*/
			lua_pushstring(L, "error");
			moon_pusherror(L, err);
			g_error_free(err);
    		if (lua_pcall(L, 2, 0, 0) != 0)
    			g_warning("error running Handle callback on G_IO_STATUS_ERROR during write: %s",
    					lua_tostring(L, -1));
			return FALSE;
			break;
	}
	return TRUE;
}/*}}}*/
static inline gboolean on_event_real(GIOChannel *ch, GIOCondition cond, gpointer data)/*{{{*/
{
	g_return_val_if_fail((cond & G_IO_NVAL) == 0, FALSE);
	g_return_val_if_fail((cond & G_IO_ERR) == 0, FALSE);

	Handle *h = data;
	if (cond & G_IO_HUP) {
		LuaState *L = h->L;
		moon_pushref(L, h->callback);
		/*lua_getfield(L, LUA_REGISTRYINDEX, "HandleTable");
		lua_pushlightuserdata(L, h);
		lua_gettable(L, -2);
		lua_remove(L, -2); // remove "HandleTable"*/
		lua_pushstring(L, "hup");
    	if (lua_pcall(L, 1, 0, 0) != 0)
    		g_warning("error running Handle callback for hup event: %s",
    				lua_tostring(L, -1));
    	return FALSE;
	}
	
	if (cond & G_IO_IN)
		if (!on_input(h))
			return FALSE;

	if (cond & G_IO_OUT && !g_queue_is_empty(h->queue))
		if (!on_output(h))
			return FALSE;

	return TRUE;
}/*}}}*/


static gboolean on_event(GIOChannel *ch, GIOCondition cond, gpointer data)/*{{{*/
{
	Handle *h = data;
	h->alive = on_event_real(ch, cond, data);
	return h->alive;
}/*}}}*/

/* }}} */

/* Methods {{{ */
static int Handle_new(LuaState *L)
{
	int fd       = luaL_checkinteger(L, 1);
	int callback = moon_ref(L, 2);

	Handle *h = moon_newclass(L, "Handle", sizeof(Handle));

	h->L        = L;
	h->queue    = g_queue_new();
	h->channel  = g_io_channel_unix_new(fd);
	h->callback = callback;
	g_io_channel_set_encoding(h->channel, NULL, NULL);
	g_io_channel_set_buffered(h->channel, FALSE);
	g_io_channel_set_flags(h->channel, G_IO_FLAG_NONBLOCK, NULL);
	h->tag = g_io_add_watch(h->channel, G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_event, h);
	h->closed = FALSE;
	h->alive  = TRUE;

	/*lua_getfield(L, LUA_REGISTRYINDEX, "HandleTable");
	lua_pushlightuserdata(L, h);
	lua_pushvalue(L, -3);
	lua_settable(L, -3);
	lua_pop(L, 1);
*/
	return 1;
}

static int Handle_write(LuaState *L)
{
	Handle *h       = moon_checkclass(L, "Handle", 1);
	if (h->alive && !h->closed) {
		const char *str = luaL_checkstring(L, 2);
		g_queue_push_tail(h->queue, g_string_new(str));
	}
	return 0;
}

static void each_g_string(GString *msg, UNUSED gpointer data)
{
	g_string_free(msg, TRUE);
}

static int Handle_close(LuaState *L)
{
	Handle *h = moon_checkclass(L, "Handle", 1);

	if (h->closed)
		return 0;

	moon_unref(L, h->callback);
	g_queue_foreach(h->queue, (GFunc)each_g_string, NULL);
	g_queue_free(h->queue);
	g_source_remove(h->tag);
	g_io_channel_shutdown(h->channel, TRUE, NULL);
	g_io_channel_unref(h->channel);
	h->closed = TRUE;
	return 0;
}

static int Handle_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Handle", 1));
  	lua_pushfstring(L, "Handle (%s)", buff);
  	return 1;
}
/* }}} */

static const LuaLReg Handle_methods[] = {
	{"new", Handle_new},
	{"write", Handle_write},
	{"close", Handle_close},
	{0, 0}
};

static const LuaLReg Handle_meta[] = {
	{"__gc", Handle_close},
	{"__tostring", Handle_tostring},
	{0, 0}
};

int luaopen_handle(LuaState *L)
{
	moon_weaktable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "HandleTable");
	moon_class_register(L, "Handle", Handle_methods, Handle_meta);
	return 1;
}
