/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"

/* Option definitions. {{{1 */
static char *hostname = "chat.haverdev.org";
static int port = 7575;

static GOptionEntry entries[] = 
{
	{ "hostname", 'H', 0, G_OPTION_ARG_STRING, &hostname, "hostname to use ", "host" },
	{ "port", 'p', 0, G_OPTION_ARG_INT, &port, "connect to port P", "P" },
	{ NULL }
};
/* }}} */

GIOChannel *std

static gboolean on_input(UNUSED GIOChannel *src, GIOCondition cond, gpointer data)
{
	LuaState *L = data;
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



int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context = g_option_context_new ("- a console haver client");
	g_option_context_add_main_entries (context, entries, NULL);
	//g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free(context);

	LuaState *L = lua_open();
	luaL_openlibs(L);
	applib_open(L);
	Entrylib_open(L);
	Bufferlib_open(L);
	netlib_open(L);
	moon_loader_init(L);

	GMainLoop *loop   = g_main_loop_new(NULL, FALSE);
	GIOChannel *input = g_io_channel_unix_new(fileno(stdin));

	moon_require(L, "boot");

	lua_close(L);
	return 0;
}

/*
	GConn *conn = gnet_conn_new(hostname, port, on_conn, L);
	gnet_conn_connect (conn);
	gnet_conn_set_watch_error (conn, TRUE);
	gnet_conn_timeout (conn, 30000);
	*/
	

#if 0
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
#endif
