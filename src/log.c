#include "moonshine/config.h"
#include "moonshine/log.h"
#include <glib.h>

static MSLogItem *make_item
	(const gchar *log_domain,
	 GLogLevelFlags log_level,
	 const gchar *message,
	 UNUSED gpointer unused_data)
{
	MSLogItem *item = g_new0(MSLogItem, 1);
	item->log_domain = g_strdup(log_domain);
	item->log_level  = log_level;
	item->message    = g_strdup(message);
	return item;
}

static void free_item(gpointer data)
{
	MSLogItem *item = data;
	g_free(item->log_domain);
	g_free(item->message);
	g_free(item);
}

MSLog *ms_log_new(void)
{
	MSLog *log = g_new0(MSLog, 1);
	log->buffer = g_sequence_new(free_item);

	return log;
}

static void log_handler
	(const gchar *log_domain,
	 GLogLevelFlags log_level,
	 const gchar *message,
	 gpointer ud)
{
	MSLog *log = (MSLog *)ud;

	g_sequence_append(log->buffer, make_item(log_domain, log_level, message, NULL));
}

void ms_log_install(MSLog *log)
{
	g_log_set_default_handler(log_handler, (gpointer) log);
}

static void unwind_each_item(gpointer data, UNUSED gpointer ud)
{
	MSLogItem *item = data;
	g_log_default_handler(item->log_domain, item->log_level, item->message, NULL);
}

void ms_log_unwind(MSLog *log)
{
	g_log_set_default_handler(g_log_default_handler, NULL);
	g_sequence_foreach(log->buffer, unwind_each_item, NULL);

	// remove everything
	g_sequence_remove_range( g_sequence_get_begin_iter(log->buffer), g_sequence_get_end_iter(log->buffer));
}

void ms_log_free(MSLog *log)
{
	ms_log_unwind(log);
	g_sequence_free(log->buffer);
	g_free(log);
}

