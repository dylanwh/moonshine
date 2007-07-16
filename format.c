/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80 cindent: */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "format.h"
#include "buffer.h"
#include "term.h"

/* TODO: remove dependence on glib */

GString *format_parse(gchar *input, ...) {
	GString *out = g_string_sized_new(strlen(input));
	gchar *p = input;

	while (1) {
		gchar *oldp = p;
		gchar *nextesc = strchr(p, '%');
		if (!nextesc) {
			g_string_append(out, p);
			return out;
		}
		g_string_append_len(out, p, nextesc - p);
		switch (*(nextesc + 1)) {
			case '%':
				g_string_append_c(out, '%');
				p = nextesc + 2;
				break;
			case '|':
				g_string_append(out, INDENT_MARK_UTF);
				p = nextesc + 2;
				break;
			case '1' ... '9':
				{
					char tb[2] = { *(nextesc + 1), 0 };
					int n      = atoi(tb);
					gchar *str = NULL;
					va_list ap;
					va_start(ap, input);
					for (int i = 0; i < n; i++) {
						str = va_arg(ap, gchar *);
						if (!str) break;
					}
					if (str) {
						g_string_append(out, str);
						p = nextesc + 2;
						break;
					} else { goto unknown_esc; }
				}
			case '{':
				{
					gchar *start = nextesc + 2;
					gchar *end = strchr(start, '}');
					if (end) {
						gchar name[end - start + 1];
						memcpy(name, start, sizeof name - 1);
						name[sizeof name - 1] = '\0';
						g_string_append(out, name); //term_color_to_utf8(name));
						p = end + 1;
						break;
					} else { goto unknown_esc; }
				}
			default:
unknown_esc:
				g_string_append_c(out, *nextesc);
				p = nextesc + 1;
				break;
		}
		g_assert(p > oldp);
	}
	g_assert_not_reached();
}

GString *format_escape(gchar *input) {
	GString *out = g_string_sized_new(strlen(input));

	gchar *p = input;
	while (1) {
		gchar *nextesc = strchr(p, '%');
		if (!nextesc) {
			g_string_append(out, p);
			return out;
		}
		g_string_append_len(out, p, nextesc - p);
		g_string_append(out, "%%");
		p = nextesc + 1;
	}
	g_assert_not_reached();
}
