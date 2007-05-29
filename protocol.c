/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80 */
 
#include "protocol.h"

GString *haver_strerror(HaverError err) {
	switch(err) {
		case HAVER_NO_ERROR: return g_string_new("No error");
		case HAVER_UNKNOWN_ESCAPE: return g_string_new("Unknown escape sequence");
		case HAVER_TRUNCATED_ESCAPE: return g_string_new("Truncated escape sequence");
		case HAVER_TRUNCATED_INPUT: return g_string_new("Truncated input");
		case HAVER_BAD_UTF8: return g_string_new("Bad utf8 sequence in input");
		case HAVER_BAD_CHARS: return g_string_new("Illegal characters in input");
		case HAVER_EMPTY_ARR: return g_string_new("Empty list of strings in encode");
		default: g_assert_not_reached();
	}
}

GPtrArray *haver_decode(const gchar *line, const gchar **remain, int *error) {
	g_assert(line);

	int dummy_err;
	const gchar *dummy_remain;
	GPtrArray *temp_a = NULL;
	GString *cur_s = NULL;
	if (!error) error = &dummy_err;
	if (!remain) remain = &dummy_remain;
	*error = HAVER_NO_ERROR;

	if (!g_utf8_validate(line, -1, NULL)) {
		*error = HAVER_BAD_UTF8;
		return NULL;
	}

	temp_a = g_ptr_array_new();
	while (*line) {
		if (!cur_s) {
			cur_s = g_string_new("");
			g_ptr_array_add(temp_a, cur_s);
		}
		switch (*line) {
			case '\n':
			case '\r':
				while (*line == '\n' || *line == '\r') line++;
				*remain = line;
				return temp_a;
			case '\t':
				cur_s = NULL;
				break;
			case '\x1F':
				line++;
				switch (*line) {
					case '\0':
					case '\t':
					case '\n':
					case '\r':
						*error = HAVER_TRUNCATED_ESCAPE;
						goto error_out;
					case 'n': g_string_append_c(cur_s, '\n'); break;
					case 'r': g_string_append_c(cur_s, '\r'); break;
					case 't': g_string_append_c(cur_s, '\t'); break;
					case 'e': g_string_append_c(cur_s, '\x1F'); break;
					default:
						*error = HAVER_UNKNOWN_ESCAPE;
						goto error_out;
				}
				break;
			default:
				g_string_append_c(cur_s, *line);
		}
		line++;
	}
	*error = HAVER_TRUNCATED_INPUT;
	goto error_out;

error_out:
	g_assert(*error != HAVER_NO_ERROR);
	void free_one(gpointer str, UNUSED gpointer user_data) {
		g_string_free(str, TRUE);
	}

	if (temp_a) {
		g_ptr_array_foreach(temp_a, free_one, NULL);
		g_ptr_array_free(temp_a, TRUE);
	}
	return NULL;
}

struct enc_info {
	int *error;
	GString *accum;
};

GString *haver_encode(const GPtrArray *line, int *error) {
	g_assert(line);

	int dummy_error;
	if (!error) error = &dummy_error;
	*error = HAVER_NO_ERROR;

	int est_size = 2;
	GString *str = NULL;

	if (line->len == 0) {
		*error = HAVER_EMPTY_ARR;
		goto error_out;
	}

	void tally(gpointer v_str, gpointer v_est) {
		int *est = v_est;
		GString *str = v_str;
		est += str->len + 1;
	}
	g_ptr_array_foreach((GPtrArray *)line, tally, &est_size);

	str = g_string_sized_new(est_size);
	void append(gpointer v_str, gpointer v_info) {
		GString *str = v_str;
		struct enc_info *info = v_info;

		if (*info->error) return;
		if (!g_utf8_validate(str->str, -1, NULL)) {
			*info->error = HAVER_BAD_UTF8;
			return;
		}

		gchar *p = str->str;
		while (*p) {
			switch (*p) {
				case '\n': g_string_append(info->accum, "\x1Fn"); break;
				case '\r': g_string_append(info->accum, "\x1Fr"); break;
				case '\t': g_string_append(info->accum, "\x1Ft"); break;
						   /* XXX: \x1Fe breaks here, is there a less ugly
							* way for it to be interpreted properly?
							*/
				case '\x1F': g_string_append(info->accum, "\x1F" "e"); break;
				default: g_string_append_c(info->accum, *p); break;
			}
			p++;
		}
		g_string_append_c(info->accum, '\t');
	}
	struct enc_info info = { error, str };
	g_ptr_array_foreach((GPtrArray *)line, append, &info);
	if (*error != HAVER_NO_ERROR)
		goto error_out;

	g_string_truncate(str, str->len - 1);
	g_string_append(str, "\x0D\x0A");

	return str;
error_out:
	g_assert(*error != HAVER_NO_ERROR);
	
	if (str) g_string_free(str, TRUE);
	return NULL;
}
