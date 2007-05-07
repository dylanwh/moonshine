#include <slang.h>
#include "haver.h"
#include "ui.h"

HaverUI *haver_ui_new(void)
{
	HaverUI *ui = g_new(HaverUI, 1);
	ui->topic   = g_string_new("");
	ui->buffer  = NULL; // GList of GStrings
	ui->entry   = NULL; // GList of gchars
	return ui;
}

void haver_ui_free(HaverUI *ui)
{
	void each(GString *s, UNUSED gpointer unused) { g_string_free(s, TRUE) }
	g_string_free(ui->topic, TRUE);
	g_list_foreach(ui->buffer, (GFunc)each, NULL);
	g_list_free(ui->buffer);
	g_list_free(ui->entry);
}


