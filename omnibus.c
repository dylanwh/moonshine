#include <glib.h>
#include "omnibus.h"

struct _OmniBus {
	GHashTable *table;
	int refcnt;
};

OmniBus *omnibus_new(void)
{
	OmniBus *bus = g_new(OmniBus, 1);
	bus->table = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify)closure_unref);
	bus->refcnt = 1;
	return bus;
}

void omnibus_bind(OmniBus *bus, const char *name, Closure *c)
{
	gpointer key = GINT_TO_POINTER(g_quark_from_string(name));
	GSList *list = g_hash_table_lookup(bus->table, key);
	closure_ref(c);
	g_hash_table_insert(bus->table, key, g_slist_prepend(list, c));
}

void omnibus_call(OmniBus *bus, const char *name, gpointer arg)
{
	g_print("Calling %s\n", name);
	gpointer key = GINT_TO_POINTER(g_quark_from_string(name));
	GSList *list = g_hash_table_lookup(bus->table, key);
	while (list) {
		Closure *c = list->data;
		closure_call(c, arg);
		list = g_slist_next(list);
	}
}

OmniBus *omnibus_ref(OmniBus *bus)
{ 
	g_return_val_if_fail (bus != NULL, NULL);
	g_return_val_if_fail (bus->refcnt > 0, bus);

	g_atomic_int_add (&bus->refcnt, 1);
	return bus;
}

static void omnibus_free(OmniBus *bus)
{
	g_hash_table_destroy(bus->table);
	g_free(bus);
}

void omnibus_unref(OmniBus *bus)
{
  	g_return_if_fail (bus != NULL);
  	g_return_if_fail (bus->refcnt > 0);

  	if (g_atomic_int_exchange_and_add (&bus->refcnt, -1) - 1 == 0)
  		omnibus_free(bus);
}

