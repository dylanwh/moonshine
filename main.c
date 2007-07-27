/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"
#include "entry.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <gnet.h>

extern const char lua_embed_boot[];

static char *user_boot_path = NULL;
static char *hostname = "chat.haverdev.org";
static int port = 7575;

static GOptionEntry entries[] = 
{
	{ "hostname", 'H', 0, G_OPTION_ARG_STRING, &hostname, "hostname to use ", "host" },
	{ "port", 'p', 0, G_OPTION_ARG_INT, &port, "connect to port P", "P" },
	{ "boot", 'b', 0, G_OPTION_ARG_STRING, &user_boot_path, "LUA bootstrap code to use", "bootfile" },
	{ NULL }
};

static int boot_lua(lua_State *L) {
	if (user_boot_path) {
		if (!luaL_dofile(L, user_boot_path))
			return 0;
		else
			lua_pop(L, 1);
	}
	char *userhome = getenv("HOME");
	if (userhome) {
		static const char suffix[] = "/.moonshine/boot.lua";
		char fullpath[strlen(userhome) + sizeof(suffix) + 1];
		strcpy(fullpath, userhome);
		strcat(fullpath, suffix);
		if (!luaL_dofile(L, fullpath))
			return 0;
		else
			lua_pop(L, 1);
	}
	if (!luaL_dofile(L, "/usr/share/moonshine/boot.lua"))
		return 0;
	else
		lua_pop(L, 1);
	/* (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0)) */
	if (luaL_loadbuffer(L, lua_embed_boot, strlen(lua_embed_boot), "embedded boot.lua")) {
		const char *err = lua_tostring(L, -1);
		g_error("BUG: Cannot load in-core boot.lua: %s", err);
		exit(EXIT_FAILURE);
	}
	int ret = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (ret == 0) {
		return 0;
	} else {
		const char *err = lua_tostring(L, -1);
		g_error("BUG: Boot from in-core boot.lua failed after pcall: %s", err);
		exit(EXIT_FAILURE);
	}
}

static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer arg)
{
	LuaState *L = arg;
	if (cond & G_IO_IN) {
		do {
			gunichar c = term_getkey();
			char buf[8];
			for (int i = 0; i < sizeof(buf); i++)
				buf[i] = 0;
			g_unichar_to_utf8(c, buf);
			moon_call(L, "on_input", "s", buf);
		} while (SLang_input_pending(1));
		//moon_call(L, "on_input_reset", "");
		return TRUE;
	}
	return FALSE;
}

static void on_conn (GConn *conn, GConnEvent *event, gpointer data)
{
	LuaState *L = data;

	switch (event->type)
	{
	  	case GNET_CONN_CONNECT:
	  	    {
	  	      	gnet_conn_timeout (conn, 0);	/* reset timeout */
	  	      	gnet_conn_readline (conn);
	  	      	moon_call(L, "on_connect", "si", hostname, port);
	  	      	break;
	  	    }
	  	case GNET_CONN_READ:
	  	    {
	  	      	/* Write line out */
	  	      	moon_call(L, "on_read", "s", event->buffer);
	  	      	gnet_conn_readline (conn);
	  	      	break;
	  	    }
	  	case GNET_CONN_WRITE:
	  	    {
	  	      	; /* do nothing */
	  	      	break;
	  	    }

	  	case GNET_CONN_CLOSE:
	  	    {
	  	      	gnet_conn_delete (conn);
	  	      	moon_call(L, "on_close", "si", hostname, port);
	  	      	break;
	  	    }

	  	case GNET_CONN_TIMEOUT:
	  	    {
	  	      	gnet_conn_delete (conn);
	  	      	moon_call(L, "on_timeout", "si", hostname, port);
	  	      	break;
	  	    }

	  	case GNET_CONN_ERROR:
	  	    {
	  	      	gnet_conn_delete (conn);
	  	      	moon_call(L, "on_error", "si", hostname, port);
	  	      	break;
	  	    }
	  	default:
	  	    g_assert_not_reached ();
	}
}

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new ("- a console haver client");
	g_option_context_add_main_entries (context, entries, NULL);
	//g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, &error);

	term_init();
	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));
	LuaState *L = lua_open();
	GConn *conn = gnet_conn_new(hostname, port, on_conn, L);

	luaL_openlibs(L);
	modapp_register(L, loop);
	modEntry_register(L);
	modBuffer_register(L);

	boot_lua(L);

	g_io_add_watch(input, G_IO_IN, on_input, L);

	gnet_conn_connect (conn);
	gnet_conn_set_watch_error (conn, TRUE);
	gnet_conn_timeout (conn, 30000);

	g_main_loop_run(loop);

	term_reset();
	lua_close(L);

	return 0;
}
