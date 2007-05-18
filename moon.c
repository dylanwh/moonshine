#include <glib.h>
#include "moon.h"

struct _Moon {
	GHashTable *table;
};

Moon *moon_new(void)
{
	Moon *moon = g_new(Moon, 1);
	moon->table = g_hash_table_new(NULL, NULL);
	return moon;
}

void moon_bind(Moon *moon, const char *name, Closure *c)
{
	gpointer key = GINT_TO_POINTER(g_quark_from_string(name));
	GSList *list = g_hash_table_lookup(moon->table, key);
	closure_ref(c);
	g_hash_table_insert(moon->table, key, g_slist_prepend(list, c));
}

void moon_call(Moon *moon, const char *name, gpointer arg)
{
	g_print("Calling %s\n", name);
	gpointer key = GINT_TO_POINTER(g_quark_from_string(name));
	GSList *list = g_hash_table_lookup(moon->table, key);
	while (list) {
		Closure *c = list->data;
		closure_call(c, arg);
		list = g_slist_next(list);
	}
}
