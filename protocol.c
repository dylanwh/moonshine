#include "config.h"
#include "protocol.h"

GSList *haver_parse(GString *s)
{
	GSList *result = NULL;
	guint start = 0;
	gchar *str = s->str;

	for (guint i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\t') {
			GString *buf = g_string_new_len(&str[start], i - start);
			result = g_slist_prepend(result, buf);
			start = i+1;
		} else if (str[i+1] == '\0') {
			GString *buf = g_string_new_len(&str[start], i + 1 - start);
			result = g_slist_prepend(result, buf);
		}
	}
	
	return g_slist_reverse(result);
}

GString *haver_format(GSList *msg)
{
	GString *buf = g_string_new("");
	void each(GString *str, GString *buf) {
		if (buf->len != 0)
			g_string_append_c(buf, '\t');
		g_string_append(buf, str->str);
	}
	g_slist_foreach(msg, (GFunc) each, buf);
	return buf;
}

void haver_msg_free(GSList *msg)
{
	void each(GString *str, UNUSED int foo)
	{
		g_string_free(str, TRUE);
	}
	g_slist_foreach(msg, (GFunc)each, NULL);
	g_slist_free(msg);
}
