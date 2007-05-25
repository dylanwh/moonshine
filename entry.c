#include "entry.h"

struct Entry {
	GList *head;
	GList *view;
};

Entry *entry_new(lua_State *L)
{
	Entry *e = g_new(Entry, 1);
	e->text  = g_string_new();
	e->start = 0;
	e->cursor = 0;
}

void entry_free(Entry *)
{

}
