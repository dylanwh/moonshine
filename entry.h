/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */

#ifndef __MOONSHINE_ENTRY_H__
#define __MOONSHINE_ENTRY_H__
#include <glib.h>
#include <lua.h>

typedef struct Entry Entry;

Entry *entry_new(void);

void entry_free(Entry *);

void entry_keypress(Entry *, gunichar uc) __attribute__((nonnull));
void entry_move(Entry *, int offset) __attribute__((nonnull));
void entry_move_to(Entry *, int absolute) __attribute__((nonnull));
gchar *entry_get(Entry *) __attribute__((nonnull));
void entry_clear(Entry *) __attribute__((nonnull));

void entry_erase(Entry *, int) __attribute__((nonnull));
void entry_erase_region(Entry *, int start, int end) __attribute__((nonnull));

void entry_render(Entry *, guint lmargin) __attribute__((nonnull));

#endif

