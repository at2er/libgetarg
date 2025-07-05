#include "getarg.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_OPTS_END(OPT) ((OPT).long_name == NULL\
		&& (OPT).short_name == '\0')

struct long_opt_node;
struct long_opt_node {
	struct option *opt;
	struct long_opt_node *children[UCHAR_MAX + 1];
};

static getarg_opt_parser default_arg_parser = NULL;
static struct option *short_opts[UCHAR_MAX + 1];
static struct long_opt_node *long_opts[UCHAR_MAX + 1];

static int apply_opt(int argc, char *argv[], struct option *opt,
		struct option *opts);
static int apply_opt_queue(int argc, char *argv[], int queue_len,
		struct option **queue);
static int init(struct option *opts);
static int init_long_opt(struct option *opt);
static struct option *long_opt_find(char *name);
static int parse_list_arg(int argc, char *argv[], struct option *opt);
static int parse_long_opt(int argc, char *argv[], struct option *opts);
static int parse_opt(int argc, char *argv[], struct option *opts);
static int parse_short_opt(int argc, char *argv[], struct option *opts);

int apply_opt(int argc, char *argv[], struct option *opt, struct option *opts)
{
	if (opt == NULL)
		goto err_unknown_opt;
	switch (opt->type) {
	case GETARG_HELP_OPT:
		getarg_help_opt(opt, opts);
		return 0;
		break;
	case GETARG_LIST_ARG:
		return parse_list_arg(argc - 1, &argv[1], opt);
		break;
	case GETARG_NO_ARG:
		if (opt->parse(0, NULL, opt))
			return -1;
		return 0;
		break;
	}
	if (argv[1][0] == '-') {
		if (!opt->is_optional_arg)
			return -1;
		if (opt->parse(0, NULL, opt))
			return -1;
		return 0;
	}
	if (opt->parse(1, &argv[1], opt))
		return -1;
	return 0;
err_unknown_opt:
	fprintf(stderr, "libgetarg: Unknown option: \"%c:%s\"\n",
			opt->short_name, opt->long_name);
	return -1;
}

int apply_opt_queue(int argc, char *argv[], int queue_len,
		struct option **queue)
{
	for (int i = 0; i < queue_len; i++) {
		if (queue[i]->parse(0, NULL, queue[i]))
			return -1;
	}
	return 0;
}

int init(struct option *opts)
{
	int i = 0;
	for (; !CHECK_OPTS_END(opts[i]); i++) {
		if (opts[i].short_name == '\0')
			continue;
		if (opts[i].long_name != NULL)
			if (init_long_opt(&opts[i]))
				return 1;
		short_opts[(uint8_t)opts[i].short_name] = &opts[i];
	}
	default_arg_parser = opts[i].parse;
	return 0;
}

int init_long_opt(struct option *opt)
{
	uint8_t index = (uint8_t)opt->long_name[0];
	struct long_opt_node *cur = long_opts[index];
	if (cur == NULL) {
		cur = malloc(sizeof(*cur));
		long_opts[index] = cur;
	}
	for (int i = 1, len = strlen(opt->long_name); i < len; i++) {
		index = opt->long_name[i];
		if (cur->children[index] == NULL)
			cur->children[index] = malloc(sizeof(*cur));
		cur = cur->children[index];
	}
	cur->opt = opt;
	return 0;
}

struct option *long_opt_find(char *name)
{
	struct long_opt_node *cur = long_opts[(uint8_t)name[2]];
	int index = 0;
	if (cur == NULL)
		return NULL;
	for (int i = 3, len = strlen(name); i < len; i++) {
		index = name[i];
		if (cur->children[index] == NULL)
			cur->children[index] = malloc(sizeof(*cur));
		cur = cur->children[index];
	}
	if (cur == NULL)
		return NULL;
	return cur->opt;
}

int parse_list_arg(int argc, char *argv[], struct option *opt)
{
	int len = 0;
	for (len = 0; len < argc; len++) {
		if (argv[len][0] == '-')
			break;
	}
	if (len == 0) {
		if (!opt->is_optional_arg)
			goto err_arg_not_found;
		if (opt->parse(0, NULL, opt))
			return -1;
		return 0;
	}
	if (opt->parse(len, argv, opt))
		return -1;
	return len;
err_arg_not_found:
	fprintf(stderr, "libgetarg: Arguments for option '%s:%c' not found!\n",
			opt->long_name, opt->short_name);
	return -1;
}

int parse_long_opt(int argc, char *argv[], struct option *opts)
{
	return apply_opt(argc, argv, long_opt_find(argv[0]), opts);
}

int parse_opt(int argc, char *argv[], struct option *opts)
{
	if (argv[0][1] == '-')
		return parse_long_opt(argc, argv, opts);
	return parse_short_opt(argc, argv, opts);
}

int parse_short_opt(int argc, char *argv[], struct option *opts)
{
	int apply_queue_len = 0;
	struct option **apply_queue = NULL, *tmp = NULL;
	if (argv[0][2] == '\0')
		return apply_opt(argc, argv, short_opts[(uint8_t)argv[0][1]],
				opts);
	apply_queue_len = strlen(argv[0]) - 1;
	apply_queue = malloc(apply_queue_len * sizeof(*apply_queue));
	for (int i = 1; argv[0][i] != '\0'; i++) {
		if ((tmp = short_opts[(uint8_t)argv[0][i]]) == NULL)
			goto err_unknown_opt;
		if (!tmp->is_optional_arg && tmp->type == GETARG_LIST_ARG)
			goto err_multiple_short_opt_list;
		apply_queue[i - 1] = tmp;
	}
	if (apply_opt_queue(argc, argv, apply_queue_len, apply_queue))
		goto err_apply_failed;
	free(apply_queue);
	return 0;
err_unknown_opt:
	free(apply_queue);
	fprintf(stderr, "libgetarg: Unknown short option: \"%s\"\n", argv[0]);
	return -1;
err_multiple_short_opt_list:
	free(apply_queue);
	fprintf(stderr, "libgetarg: Multiple short option,"
			" but need list arguments: \"%s\"\n",
			argv[0]);
	return -1;
err_apply_failed:
	free(apply_queue);
	fprintf(stderr, "libgetarg: Apply short option failed: \"%s\"\n",
			argv[0]);
	return -1;
}

int getarg(int argc, char *argv[], struct option *opts)
{
	int default_argc = 0;
	char **default_argv = NULL;
	int i, jump;
	if (init(opts))
		goto err_init_failed;
	for (i = 1, jump = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			if ((jump = parse_opt(argc - i, &argv[i], opts)) == -1)
				goto err_free_default_argv;
			i += jump;
			jump = 0;
			continue;
		}
		default_argc++;
		default_argv = realloc(default_argv,
				default_argc * sizeof(char*));
		default_argv[default_argc - 1] = argv[i];
	}
	if (default_argv == NULL)
		return 0;
	if (default_arg_parser == NULL)
		goto err_default_arg_parser_not_exist;
	if (default_arg_parser(default_argc, default_argv, opts))
		goto err_free_default_argv;
	free(default_argv);
	return 0;
err_init_failed:
	fprintf(stderr, "libgetarg: Init failed!\n");
	return 1;
err_default_arg_parser_not_exist:
	fprintf(stderr, "libgetarg: "
			"Default arguments parser doesn't exist!\n");
	return 1;
err_free_default_argv:
	if (default_argv == NULL)
		return 1;
	free(default_argv);
	return 1;
}

void getarg_help_opt(struct option *opt, struct option *opts)
{
	printf("Usage: [OPTIONS] ");
	if (opt != NULL && opt->parse != NULL)
		opt->parse(0, NULL, opt);
	printf("\n\nOPTIONS:\n");
	for (int i = 0; !CHECK_OPTS_END(opts[i]); i++) {
		printf("\t-%c, --%s:\t%s\n",
				opts[i].short_name,
				opts[i].long_name,
				opts[i].document);
		if (opts[i].opt_doc != NULL)
			printf("\t\t%s\n", opts[i].opt_doc);
	}
	printf("\n");
	if (opt != NULL && opt->parse != NULL)
		opt->parse(1, NULL, opt);
	exit(0);
}
