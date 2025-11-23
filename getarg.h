/**
 * SPDX-License-Identifier: MIT
 *
 * usage
 * - compiling:
 *     CFLAGS+=[-L<getarg dir>] -lgetarg
 *
 * - define options:
 *     ```
 *     static struct option opts[] = {
 *       // OPT_FLAG
 *       OPT_FLAG("flag-a-long-name", 'a', &flags, 1),
 *       OPT_FLAG("flag-b-long-name", 'b', &flags, 1 << 1),
 *
 *       // OPT_HELP
 *       // @param usages: const char * [], see `Help (Usages)` for more
 *       OPT_HELP("help", 'h', usages),
 *
 *       // OPT_STRING
 *       // @param str: char ** (point to 'char *')
 *       OPT_STRING("string", 's', &str),
 *
 *       // OPT_UINT
 *       // @param uint: uint64_t * (point to 'uint64_t')
 *       OPT_UINT("uint", 'u', &uint),
 *
 *       OPT_END
 *     }
 *     ```
 *
 * - help (usage):
 *     ```
 *     static const char *usages[] = {
 *     "usage: getarg: [OPTIONS]...",
 *     "",
 *     "options:",
 *     "...OPTIONS...",
 *     NULL
 *     };
 *
 *     // In options definition:
 *     {
 *       ...
 *       OPT_HELP("help", 'h', usages),
 *       ...
 *     }
 *     ```
 *
 * - parsing:
 *     ```in 'main()'
 *     enum GETARG_RESULT ret;
 *
 *     GETARG_BEGIN(ret, argc, argv, opts) {
 *     case GETARG_RESULT_SUCCESSFUL: break;
 *     case GETARG_RESULT_UNKNOWN:    GETARG_SHIFT(argc, argv); break;
 *     // ... handle more result (enum GETARG_RESULT)
 *     default: return 1;
 *     } GETARG_END;
 *     ```
 */
#ifndef LIBGETARG_H
#define LIBGETARG_H
#include <stddef.h>
#include <stdint.h>

#define NO_LONG_NAME NULL
#define NO_SHORT_NAME '\0'

enum GETARG_OPT_TYPE {
	GETARG_OPT_END,

	GETARG_OPT_FLAG, /* --enable-xxx without any argument */
	GETARG_OPT_HELP,
	GETARG_OPT_STRING,
	GETARG_OPT_UINT /* uint64_t */
};

enum GETARG_RESULT {
	GETARG_RESULT_END,

	GETARG_RESULT_DASH,
	GETARG_RESULT_DOUBLE_DASH,
	GETARG_RESULT_OPT_NOT_FOUND,
	GETARG_RESULT_PARSE_ARG_FAILED,
	GETARG_RESULT_SUCCESSFUL,
	GETARG_RESULT_UNKNOWN
};

struct option {
	enum GETARG_OPT_TYPE type;
	const char *long_name;
	const char short_name;
	void *value;
	uintptr_t data;
};

#define OPT_FLAG(LN, SN, FLAGS, FLAG_BIT) \
	{GETARG_OPT_FLAG, LN, SN, FLAGS, FLAG_BIT}
#define OPT_HELP(LN, SN, USAGES) {GETARG_OPT_HELP,   LN, SN, USAGES, 0}
#define OPT_STRING(LN, SN, STR)  {GETARG_OPT_STRING, LN, SN, STR,    0}
#define OPT_UINT(LN, SN, UINT)   {GETARG_OPT_UINT,   LN, SN, UINT,   0}
#define OPT_END {GETARG_OPT_END, NO_LONG_NAME, NO_SHORT_NAME, NULL,  0}

#define GETARG_BEGIN(RESULT, ARGC, ARGV, OPTS) do { \
	GETARG_SHIFT(ARGC, ARGV); \
	while (((RESULT) = getarg(&(ARGC), &(ARGV), (OPTS))) != GETARG_RESULT_END) { \
		switch ((RESULT))
#define GETARG_END \
	}} while (0)
#define GETARG_SHIFT(ARGC, ARGV) do { (ARGC)--; (ARGV)++; } while (0)

enum GETARG_RESULT getarg(int *argc, char **argv[], struct option *opts);

#endif
