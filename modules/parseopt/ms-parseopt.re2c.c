#include "ms-parseopt.h"
#include <string.h>
#include <stdlib.h>
#include <glib.h>

struct token {
	GString *str;
	const char *start;
};

struct parsectx {
	const char *p;
	struct token lookahead[2];
	size_t lookahead_sz;
	int all_literal;
};

static void initctx(const char *optstr, struct parsectx *pc) {
	pc->p = optstr;
	pc->lookahead_sz = 0;
	pc->all_literal  = 0;
	for (int i = 0; i < 2; i++)
		pc->lookahead[i].str = g_string_new("");
}

static void freectx(struct parsectx *pc) {
	for (size_t i = 0; i < 2; i++)
		g_string_free(pc->lookahead[i].str, 1);
}

static void consume_token(struct parsectx *pc) {
	GString *temp = pc->lookahead[0].str;
	g_assert(pc->lookahead_sz);
	pc->lookahead_sz--;
	pc->lookahead[0] = pc->lookahead[1];
	pc->lookahead[1].str = temp;
}

static char escape(char e) {
	switch (e) {
		case 'r' : return '\r';
		case 'n' : return '\n';
		case 't' : return '\t';
		default  : return e;
	}
}

static int parse_once(struct parsectx * restrict pc, int index) {
	struct token *t = &pc->lookahead[index];
start:
/*!re2c
	re2c:define:YYCTYPE = "unsigned char";
	re2c:define:YYCURSOR = (pc->p);
	re2c:yyfill:enable = 0;
	re2c:yych:conversion = 1;
	re2c:indent:top = 1;

	SPACE   = [ \t];
	SQUOTE  = ['];
	DQUOTE  = ["];
	any     = [\001-\377];
	eoi     = [\000];
	SLASH   = "\\";

	SPACE { goto start; }
	eoi { pc->p--; return 0; }
	any {
		pc->p--;
		goto begin_token;
	}
*/
begin_token:
	t->start = pc->p;
token_loop:
/*!re2c
    SPACE { pc->p--; return 1; }
	eoi { pc->p--; return 1; }
	SQUOTE { goto single_quote; }
	DQUOTE { goto double_quote; }
	SLASH any { g_string_append_c(t->str, escape(pc->p[-1])); goto token_loop; }
	any { g_string_append_c(t->str, pc->p[-1]); goto token_loop; }
*/
single_quote:
/*!re2c
	SQUOTE { goto token_loop; }
	eoi { pc->p--; return 1; }
	any { g_string_append_c(t->str, pc->p[-1]); goto single_quote; }
*/
double_quote:
/*!re2c
	DQUOTE { goto token_loop; }
	eoi { pc->p--; return 1; }
	SLASH any { g_string_append_c(t->str, escape(pc->p[-1])); goto double_quote; }
	any { g_string_append_c(t->str, pc->p[-1]); goto double_quote; }
*/
}

static const struct token *get_lookahead(struct parsectx *pc, size_t offset) {
	g_assert(offset < sizeof(pc->lookahead) / sizeof(pc->lookahead[0]));
	while (offset >= pc->lookahead_sz) {
		g_string_assign(pc->lookahead[pc->lookahead_sz].str, "");
		if (parse_once(pc, pc->lookahead_sz))
			pc->lookahead_sz++;
		else {
			return NULL; /* EOI */
		}
	}
	return &pc->lookahead[offset];
}

const char *ms_parseopt_run(void *baton, const char *argstr, const MSParseOptCallbacks *cb) {
	struct parsectx pc;
	initctx(argstr, &pc);

	while (1) {
		const struct token *t = get_lookahead(&pc, 0);
		int ret;
		if (!t)
			break;
		if (pc.all_literal || t->str->str[0] != '-') {
			ret = cb->literalopt(baton, t->str->str, t->start);
			if (ret != MS_PARSEOPT_STOP)
				ret = MS_PARSEOPT_NOARG;
		} else if (t->str->str[1] == '-') {
			if (t->str->str[2] == '\0') {
				pc.all_literal = 1;
			} else {
				int inline_arg;
				const char *argument;
				char *eqp;
				if ((eqp = strchr(&t->str->str[2], '='))) {
					inline_arg = 1;
					*eqp = 0;
					argument = eqp + 1;
				} else {
					const struct token *next = get_lookahead(&pc, 1);
					inline_arg = 0;
					if (next)				
						argument = next->str->str;
					else
						argument = NULL;
				}
				ret = cb->longopt(baton, &t->str->str[2], argument, t->start);
				if (ret == MS_PARSEOPT_EATARG && inline_arg)
					ret = MS_PARSEOPT_NOARG;
			}
		} else {
			ret = MS_PARSEOPT_NOARG;
			for (const char *p = &t->str->str[1]; *p; p++) {
				int inline_arg;
				const char *argp;
				if (p[1]) {
					inline_arg = 1;
					argp = &p[1];
				} else {
					inline_arg = 0;
					const struct token *argt = get_lookahead(&pc, 1);
					if (argt)
						argp = argt->str->str;
					else
						argp = NULL;
				}
				ret = cb->shortopt(baton, *p, argp, t->start);
				if (ret == MS_PARSEOPT_EATARG) {
					if (inline_arg)
						ret = MS_PARSEOPT_NOARG;
					break;
				}
			}
		}
		if (ret == MS_PARSEOPT_STOP) {
			pc.p = t->start;
			break;
		}
		consume_token(&pc);
		if (ret == MS_PARSEOPT_EATARG)
			consume_token(&pc);
	}
	freectx(&pc);
	return pc.p;
}
