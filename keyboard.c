#include <string.h>
#include <glib.h>

#include "config.h"
#include "closure.h"
#include "keyboard.h"

inline static void init_keymap(Keyboard *kb);

Keyboard *spoon_keyboard_new(void)
{
	Keyboard *kb     = g_new(Keyboard, 1);
	kb->keymap   = SLang_create_keymap("default", NULL);
	kb->table    = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)spoon_closure_unref);
	kb->channel  = g_io_channel_unix_new (fileno(stdin));
	g_io_channel_ref(kb->channel);
	
	g_assert(kb->keymap != NULL);
	init_keymap(kb);
	return kb;
}

void spoon_keyboard_free(Keyboard *kb)
{
	/* how do I free kb->keymap? */
	g_hash_table_unref(kb->table);
	spoon_closure_unref(kb->fallback);
	g_io_channel_unref(kb->channel);
}

void spoon_keyboard_define(Keyboard *kb, char *spec, char *name)
{
	g_assert(spec != NULL);
	g_assert(name != NULL);
	g_assert(*spec != '\0');
	g_assert(*name != '\0');
	SLkm_define_keysym(spec, g_quark_from_string(name), kb->keymap);
}

void spoon_keyboard_bind(Keyboard *kb, char *keyname, Closure *c)
{
	gpointer key = (gpointer) g_intern_string(keyname);
	Closure *old = (Closure *)g_hash_table_lookup(kb->table, key);
	if (old)
		spoon_closure_unref(old);

	spoon_closure_ref(c);
	g_hash_table_insert(kb->table, key, c);
}

void spoon_keyboard_bind_fallback(Keyboard *kb, Closure *c)
{
	if (kb->fallback)
		spoon_closure_unref(kb->fallback);

	spoon_closure_ref(c);
	kb->fallback = c;
}

const char *spoon_keyboard_read(Keyboard *kb)
{
	SLang_Key_Type *key = SLang_do_key(kb->keymap, (int (*)(void)) SLang_getkey);

	if (key == NULL || key->type != SLKEY_F_KEYSYM) {
		SLang_flush_input();
		return NULL;
	}
	return g_quark_to_string(key->f.keysym);
}

static gboolean on_input(
		GIOChannel *input, 
		GIOCondition cond, 
		gpointer data)
{
	if (cond & G_IO_IN) {
		Keyboard *kb = (Keyboard *)data;
		const char *s = spoon_keyboard_read(kb);
		if (s) {
			Closure *c = (Closure *) g_hash_table_lookup(kb->table, (gpointer)s);
			if (c)
				spoon_closure_call(c, (gpointer)s);
			else if (kb->fallback)
				spoon_closure_call(kb->fallback, (gpointer)s);
		}
		return TRUE;
	} else {
		g_print("stdin error!");
		return FALSE;
	}
}

void spoon_keyboard_add_watch(Keyboard *kb)
{
	g_io_add_watch(kb->channel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, on_input, kb);
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
	   	spoon_keyboard_define(kb, esc_seq, string->str);
	   	g_string_free(string, TRUE);
   	}
   	spoon_keyboard_define(kb, "^(k;)", "F10");
   	spoon_keyboard_define(kb, "^(F1)", "F11");
   	spoon_keyboard_define(kb, "^(F2)", "F12");

   	spoon_keyboard_define(kb, "^(ku)", "UP");
   	spoon_keyboard_define(kb, "^(kd)", "DOWN");
   	spoon_keyboard_define(kb, "^(kl)", "LEFT");
   	spoon_keyboard_define(kb, "^(kr)", "RIGHT");
   	spoon_keyboard_define(kb, "^(kP)", "PPAGE");
   	spoon_keyboard_define(kb, "^(kN)", "NPAGE");
   	spoon_keyboard_define(kb, "^(kh)", "HOME");
   	spoon_keyboard_define(kb, "^(@7)", "END");
   	spoon_keyboard_define(kb, "^(K1)", "A1");
   	spoon_keyboard_define(kb, "^(K3)", "A3");
   	spoon_keyboard_define(kb, "^(K2)", "B2");
   	spoon_keyboard_define(kb, "^(K4)", "C1");
   	spoon_keyboard_define(kb, "^(K5)", "C3");
   	spoon_keyboard_define(kb, "^(%0)", "REDO");
   	spoon_keyboard_define(kb, "^(&8)", "UNDO");
   	spoon_keyboard_define(kb, "^(kb)", "BACKSPACE");
   	spoon_keyboard_define(kb, "^(@8)", "ENTER");
   	spoon_keyboard_define(kb, "^(kD)", "DELETE");
}

