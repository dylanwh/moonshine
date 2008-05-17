/* vim: set ts=4 sw=4 noexpandtab cindent: */
#include "moonshine/ms-async-queue-source.h"
#include <glib.h>

typedef struct {
	GSource src;
	GAsyncQueue *queue;
} AsyncQueueSource;

static gboolean ms_async_queue_source_prepare(GSource *src_arg, gint *timeout) /*{{{*/
{
	AsyncQueueSource *src = (AsyncQueueSource *) src_arg;
	g_assert(src->queue);
	*timeout = -1;
	return g_async_queue_length(src->queue) > 0;
}/*}}}*/

static gboolean ms_async_queue_source_check(GSource *src_arg) /*{{{*/
{ 	
	AsyncQueueSource *src = (AsyncQueueSource *) src_arg;
	g_assert(src->queue);
	return g_async_queue_length(src->queue) > 0;
}/*}}}*/

static gboolean ms_async_queue_source_dispatch (/*{{{*/
		GSource *src_arg,
		GSourceFunc func_arg, 
		gpointer userdata)
{
	AsyncQueueSource *src = (AsyncQueueSource *) src_arg;
	g_assert(src->queue);
	MSAsyncQueueSourceFunc func            = (MSAsyncQueueSourceFunc) func_arg;
	gpointer data         = g_async_queue_pop(src->queue);
	return func(data, userdata);
}//}}}

static void ms_async_queue_source_finalize(GSource *src_arg)/*{{{*/
{
	AsyncQueueSource *src = (AsyncQueueSource *) src_arg;
	g_async_queue_unref(src->queue);
}/*}}}*/

static GSourceFuncs ms_async_queue_source_functions = { /*{{{*/
	ms_async_queue_source_prepare,
	ms_async_queue_source_check, 
	ms_async_queue_source_dispatch, 
	ms_async_queue_source_finalize
}; /*}}}*/

guint ms_async_queue_add_watch(GAsyncQueue *queue, MSAsyncQueueSourceFunc func, gpointer userdata, GDestroyNotify notify) /*{{{*/
{
	AsyncQueueSource *src = (AsyncQueueSource *) g_source_new(&ms_async_queue_source_functions, sizeof(AsyncQueueSource));
	src->queue            = g_async_queue_ref(queue);
	g_source_set_callback((GSource *) src, (GSourceFunc) func, userdata, notify);
	return g_source_attach((GSource *) src, NULL);
}/*}}}*/


