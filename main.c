/* test */
#include <stdint.h>
#include <stdio.h>
#define GETARG_IMPL
#include "getarg.h"

#define LENGTH(X) (sizeof((X)) / sizeof((X)[0]))

static char *long_only_arg = NULL;
static char *long_and_short_arg = NULL;
static char *short_only_arg = NULL;

static uint64_t uint = 0;
static uint64_t flags = 0;

static const char *usages[] = {
"usage: getarg: [OPTIONS]...",
"",
"options:",
"  --enable-a:                test flag a 001",
"  --enable-b:                test flag b 010",
"  --enable-c:                test flag c 100",
"  --long-only:               test long option",
"  -x, --long-and-short-arg:  test long and short option",
"  -s:                        test short option",
"  -u, --uint:                test uint option argument",
NULL
};

static struct option opts[] = {
	OPT_FLAG("enable-a", NO_SHORT_NAME,    &flags, 1),
	OPT_FLAG("enable-b", NO_SHORT_NAME,    &flags, 1 << 1),
	OPT_FLAG("enable-c", NO_SHORT_NAME,    &flags, 1 << 2),
	OPT_HELP("help",                 'h',  usages),
	OPT_STRING("long-only", NO_SHORT_NAME, &long_only_arg),
	OPT_STRING(NO_LONG_NAME,         's',  &short_only_arg),
	OPT_STRING("long-and-short-arg", 'x',  &long_and_short_arg),
	OPT_UINT("uint",                 'u',  &uint),
	OPT_END
};

int main(int argc, char *argv[])
{
	enum GETARG_RESULT ret;

	GETARG_BEGIN(ret, argc, argv, opts) {
	case GETARG_RESULT_SUCCESSFUL: break;
	case GETARG_RESULT_UNKNOWN:    GETARG_SHIFT(argc, argv); break;
	default: return 1;
	} GETARG_END;

	if (long_only_arg)
		printf("--long-only(string): %s\n", long_only_arg);
	if (long_and_short_arg)
		printf("--long-and-short-arg|-x(string): %s\n", long_and_short_arg);
	if (short_only_arg)
		printf("-s(string): %s\n", short_only_arg);
	if (uint)
		printf("--uint|-u(uint64_t): %lu\n", uint);

	if (flags & 1)
		printf("--enable-a\n");
	if (flags & 1 << 1)
		printf("--enable-b\n");
	if (flags & 1 << 2)
		printf("--enable-c\n");

	return 0;
}
