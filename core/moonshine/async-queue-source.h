#include <moonshine/config.h>
#include <glib.h>

/* This is like a GIOFunc, return false to remove the GSource. */
typedef gboolean (*MSAsyncQueueSourceFunc)(gpointer data, gpointer userdata);

guint ms_async_queue_add_watch(GAsyncQueue *queue, MSAsyncQueueSourceFunc func, gpointer userdata, GDestroyNotify notify);
