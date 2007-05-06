#include "ui.h"




HaverUI *haver_ui_new(void)
{
	HaverUI *ui = g_new(HaverUI, 1);
	ui->topic   = g_string_new("");
	ui->buffer  = NULL;
	ui->entry   = g_string_new("");
	return ui;
}

void haver_ui_free(HaverUI *ui)
{
	/* we have infinite memory, right? */
}

void haver_ui_display(HaverUI *ui, GString *msg)
{
	ui->buffer = g_slist_prepend(ui->buffer, (gpointer) msg);
}

void haver_ui_redraw(HaverUI *ui)
{

}
