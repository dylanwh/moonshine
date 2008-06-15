#ifndef CMDPARSER_H
#define CMDPARSER_H 1


typedef enum {
	MS_CMDPARSER_STOP,
	MS_CMDPARSER_NOARG,
	MS_CMDPARSER_EATARG
} MSCmdParserStatus;

/* General notes:
 *
 * argument, literal, and opt (in the case of longopt()) are owned by the caller. You
 * must strdup() them if you wish to work with them.
 *
 * start is the beginning of the option's token. Note with short arguments, this points
 * to the '-' at the start. Also, this will be a pointer directly into the string you
 * gave it, so it's valid after calling CMDPARSER_STOP.
 *
 * cb->literalopt() must never return CMDPARSER_NOARG.
 *
 * argument may also be NULL at the end of the string.
 *
 * The return value of cmdparser is a pointer to the start of the token you returned
 * CMDPARSER_STOP from; or a pointer to the end of the passed string if you let it run
 * to the end.
 */

typedef struct {
	MSCmdParserStatus (*shortopt)(void *baton, const char opt, const char *argument, const char *start);
	MSCmdParserStatus (*longopt)(void *baton, const char *opt, const char *argument, const char *start);
	MSCmdParserStatus (*literalopt)(void *baton, const char *literal, const char *start);
} cmdparser_cb;

const char *ms_cmdparser(void *baton, const char *argstr, const cmdparser_cb *cb);

#endif

