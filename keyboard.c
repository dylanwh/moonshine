#include <string.h>
#include <glib.h>
#include <stdlib.h>

#include "config.h"
#include "closure.h"
#include "keyboard.h"

inline static void init_keymap(Keyboard *kb);
static gboolean on_input(GIOChannel *input, GIOCondition cond, gpointer data);

Keyboard *keyboard_new(void)
{
	Keyboard *kb     = g_new(Keyboard, 1);
	kb->keymap   = SLang_create_keymap("default", NULL);
	kb->table    = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)closure_unref);
	kb->channel  = g_io_channel_unix_new (fileno(stdin));
	g_io_channel_ref(kb->channel);
	
	g_assert(kb->keymap != NULL);
	init_keymap(kb);
	g_io_add_watch(kb->channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, kb);

	return kb;
}

void keyboard_free(Keyboard *kb)
{
	/* how do I free kb->keymap? */
	g_hash_table_unref(kb->table);
	closure_unref(kb->fallback);
	g_io_channel_unref(kb->channel);
}

void keyboard_define(Keyboard *kb, char *spec, char *name)
{
	g_assert(spec != NULL);
	g_assert(name != NULL);
	g_assert(*spec != '\0');
	g_assert(*name != '\0');
	SLkm_define_keysym(spec, g_quark_from_string(name), kb->keymap);
}

void keyboard_bind(Keyboard *kb, char *keyname, Closure *c)
{
	gpointer key = (gpointer) g_intern_string(keyname);
	closure_ref(c);
	Closure *old = (Closure *)g_hash_table_lookup(kb->table, key);
	if (old)
		closure_unref(old);

	g_hash_table_insert(kb->table, key, c);
}

void keyboard_bind_fallback(Keyboard *kb, Closure *c)
{
	closure_ref(c);
	if (kb->fallback)
		closure_unref(kb->fallback);

	kb->fallback = c;
}

const char *keyboard_read(Keyboard *kb)
{
	SLang_Key_Type *key = SLang_do_key(kb->keymap, (int (*)(void)) SLang_getkey);

	if (key == NULL || key->type != SLKEY_F_KEYSYM) {
		SLang_flush_input();
		return NULL;
	}
	return g_quark_to_string(key->f.keysym);
}

inline static void init_keymap(Keyboard *kb)
{
   	char esc_seq[10];
   	esc_seq[1] = 0;
   	for (int i = 1; i < 256; i++) {
   		if (i == 127)
   			continue;
   	   	esc_seq[0] = (char) i;
   	   	SLkm_define_keysym(esc_seq, g_quark_from_string(esc_seq), kb->keymap);
   	}
   	strcpy (esc_seq, "^(kX)");
   	for (int i = 0; i <= 9; i++)
   	{
   		GString *string = g_string_new("");
   		g_string_printf(string, "F%d", i);
	   	esc_seq[3] = '0' + i;
	   	keyboard_define(kb, esc_seq, string->str);
	   	g_string_free(string, TRUE);
   	}
   	keyboard_define(kb, "^(k;)", "F10");
   	keyboard_define(kb, "^(F1)", "F11");
   	keyboard_define(kb, "^(F2)", "F12");

   	keyboard_define(kb, "^(ku)", "UP");
   	keyboard_define(kb, "^(kd)", "DOWN");
   	keyboard_define(kb, "^(kl)", "LEFT");
   	keyboard_define(kb, "^(kr)", "RIGHT");
   	keyboard_define(kb, "^(kP)", "PPAGE");
   	keyboard_define(kb, "^(kN)", "NPAGE");
   	keyboard_define(kb, "^(kh)", "HOME");
   	keyboard_define(kb, "^(@7)", "END");
   	keyboard_define(kb, "^(K1)", "A1");
   	keyboard_define(kb, "^(K3)", "A3");
   	keyboard_define(kb, "^(K2)", "B2");
   	keyboard_define(kb, "^(K4)", "C1");
   	keyboard_define(kb, "^(K5)", "C3");
   	keyboard_define(kb, "^(%0)", "REDO");
   	keyboard_define(kb, "^(&8)", "UNDO");
   	keyboard_define(kb, "^(kb)", "BACKSPACE");
   	keyboard_define(kb, "^(@8)", "ENTER");
   	keyboard_define(kb, "^(kD)", "DELETE");
}

static gboolean on_input(
		GIOChannel *input, 
		GIOCondition cond,
		gpointer data)
{
	if (cond & G_IO_ERR || cond & G_IO_HUP || cond & G_IO_NVAL) {
		g_print("Got IO error");
		exit(1);
	} else if (cond & G_IO_IN) {
		Keyboard *kb = (Keyboard *)data;
		const char *s = keyboard_read(kb);
		if (s) {
			Closure *c = (Closure *) g_hash_table_lookup(kb->table, (gpointer)s);
			if (c)
				closure_call(c, (gpointer)s);
			else if (kb->fallback)
				closure_call(kb->fallback, (gpointer)s);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}
