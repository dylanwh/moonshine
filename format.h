/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80 cindent: */

#ifndef __MOONSHINE_FORMAT_H__
#define __MOONSHINE_FORMAT_H__ 1

#include <glib.h>

GString *format_parse(gchar *input, ...) __attribute__((nonnull(1), sentinel));
GString *format_escape(gchar *input) __attribute__((nonnull));

#endif

