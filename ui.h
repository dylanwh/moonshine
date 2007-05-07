#ifndef _HAVER_UI_H
#define _HAVER_UI_H
#include <glib.h>

typedef struct {
	GString *topic;
	GList *buffer;
	GList *entry;
} HaverUI;

HaverUI *haver_ui_new(void);
void haver_ui_free(HaverUI *);
void haver_ui_init(void);
void haver_ui_reset(void);

#endif
