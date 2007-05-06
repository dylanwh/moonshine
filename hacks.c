#include <string.h>
#include <gc.h>
#include <glib.h>
#include "hacks.h"

static void *GC_calloc (size_t nelem, size_t size)
{
	void *p = GC_malloc (nelem * size);
	memset (p, 0, nelem * size);
	return p;
}


void hacks_init(void)
{
	GC_INIT();
	static GMemVTable table = {
		.malloc = GC_malloc,
		.realloc = GC_realloc,
		.calloc  = GC_calloc,
		.free    = GC_free,
		.try_malloc = NULL,
		.try_realloc = NULL,
	};
	g_mem_set_vtable(&table);
}
