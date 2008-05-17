#include <moonshine/config.h>
#include <glib.h>

typedef gboolean (*MSAsyncQueueSourceFunc)(gpointer data, gpointer userdata);
guint ms_async_queue_add_watch(GAsyncQueue *queue, MSAsyncQueueSourceFunc func, gpointer userdata, GDestroyNotify notify);
