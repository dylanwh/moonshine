#include <slang.h>
#include "haver.h"
#include "ui.h"
#include <ctype.h>
#include <stdlib.h>
HaverUI *haver_ui_new(void)
{
	HaverUI *ui   = g_new(HaverUI, 1);
	ui->topic     = g_string_new("");
	ui->buffer    = NULL; // GList of GStrings
	ui->entry     = g_string_sized_new(512);
	haver_ui_draw(ui);
	return ui;
}

void haver_ui_free(HaverUI *ui)
{
	void each(GString *s, UNUSED gpointer unused) { g_string_free(s, TRUE); }
	g_string_free(ui->topic, TRUE);
	g_string_free(ui->entry, TRUE);
	g_list_foreach(ui->buffer, (GFunc)each, NULL);
	g_list_free(ui->buffer);
}

void haver_ui_print(HaverUI *ui, GString *msg)
{
	ui->buffer = g_list_prepend(ui->buffer, (gpointer) msg);
}

void haver_ui_getkey(HaverUI *ui)
{
	int key = SLkp_getkey();
	switch (key) {
		case 'q': exit(0);
		default:
			if (isprint((char)key))
				g_string_append_c(ui->entry, (char)key);
	}
	haver_ui_draw(ui);
}

void haver_ui_draw(HaverUI *ui)
{
	/* write the topic */
	SLsmg_gotorc(0, 0);
	SLsmg_write_nstring(ui->topic->str, ui->topic->len);

	/* show entry text */
	SLsmg_gotorc(SLtt_Screen_Rows - 1, 0);
	SLsmg_write_nstring(ui->entry->str, ui->entry->len);

	/* finally, write to the real display */
	SLsmg_refresh();
}
