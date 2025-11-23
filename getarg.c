#include "getarg.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHIFT(ARGC_PTR, ARGV_PTR) GETARG_SHIFT(*(ARGC_PTR), *(ARGV_PTR))

#define FIND_OPT_TEMPLATE(OPTS, COND) \
	{ \
		for (int i = 0; (OPTS)[i].type != GETARG_OPT_END; i++) { \
			if (COND) \
				return &opts[i]; \
		} \
		return NULL; \
	}

static struct option *find_long_opt(char *opt, struct option *opts)
	FIND_OPT_TEMPLATE(opts, (opts[i].long_name
				&& strcmp(&opt[2], opts[i].long_name)
				== 0))

static struct option *find_short_opt(char *opt, struct option *opts)
	FIND_OPT_TEMPLATE(opts, (opts[i].short_name == opt[1]))

#undef FIND_OPT_TEMPLATE

typedef enum GETARG_RESULT result_t;

static result_t apply_help_opt(const char *usages[]);

static result_t __err_opt_not_found(int line, const char *opt);
#define err_opt_not_found(STR) __err_opt_not_found(__LINE__, STR)

static result_t parse_arg(int *argc, char **argv[], struct option *opt);
static result_t parse_long_opt(int *argc, char **argv[], struct option *opts);
static result_t parse_opt(int *argc, char **argv[], struct option *opts);
static result_t parse_short_opt(int *argc, char **argv[], struct option *opts);

result_t apply_help_opt(const char *usages[])
{
	for (int i = 0; usages[i] != NULL; i++)
		puts(usages[i]);
	return GETARG_RESULT_SUCCESSFUL;
}

result_t __err_opt_not_found(int line, const char *opt)
{
	fprintf(stderr, "[libgetarg:%s:%d]: option not found: '%s'\n",
			__FILE__,
			line,
			opt);
	return GETARG_RESULT_OPT_NOT_FOUND;
}

result_t parse_arg(int *argc, char **argv[], struct option *opt)
{
	char *arg = **argv, *tmp;
	switch (opt->type) {
	case GETARG_OPT_END:
		break;
	case GETARG_OPT_HELP:
		return apply_help_opt(opt->value);
	case GETARG_OPT_FLAG:
		*(uint64_t*)opt->value |= opt->data;
		break;
	case GETARG_OPT_STRING:
		*(char**)opt->value = arg;
		SHIFT(argc, argv);
		break;
	case GETARG_OPT_UINT:
		*((uint64_t*)opt->value) = strtoull(arg, &tmp, 10);
		if (tmp && tmp[0] != '\0')
			return GETARG_RESULT_PARSE_ARG_FAILED;
		break;
	}
	return GETARG_RESULT_SUCCESSFUL;
}

result_t parse_long_opt(int *argc, char **argv[], struct option *opts)
{
	struct option *opt;
	if ((**argv)[2] == '\0')
		return GETARG_RESULT_DOUBLE_DASH;

	if (!(opt = find_long_opt(**argv, opts)))
		return err_opt_not_found(**argv);

	SHIFT(argc, argv);
	return parse_arg(argc, argv, opt);
}

result_t parse_opt(int *argc, char **argv[], struct option *opts)
{
	if ((**argv)[1] == '-')
		return parse_long_opt(argc, argv, opts);
	return parse_short_opt(argc, argv, opts);
}

result_t parse_short_opt(int *argc, char **argv[], struct option *opts)
{
	struct option *opt;
	if ((**argv)[1] == '\0')
		return GETARG_RESULT_DASH;

	if (!(opt = find_short_opt(**argv, opts)))
		return err_opt_not_found(**argv);

	if ((**argv)[2] != '\0') {
		(**argv) = &(**argv)[2];
	} else {
		SHIFT(argc, argv);
	}
	return parse_arg(argc, argv, opt);
}

enum GETARG_RESULT getarg(int *argc, char **argv[], struct option *opts)
{
	if (*argc <= 0)
		return GETARG_RESULT_END;
	if ((**argv)[0] == '-')
		return parse_opt(argc, argv, opts);
	return GETARG_RESULT_UNKNOWN;
}
