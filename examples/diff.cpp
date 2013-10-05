#include <stdio.h>
#include <git2.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "git2cpp/threads_initializer.h"
#include "git2cpp/repo.h"

using namespace git;

Tree resolve_to_tree(Repository const & repo, const char *identifier)
{
    Object obj = revparse_single(repo, identifier);

	switch (obj.type()) 
    {
	case GIT_OBJ_TREE:
		return obj.to_tree();
		break;
	case GIT_OBJ_COMMIT:
		return obj.to_commit().tree();
		break;
	}

    std::ostringstream ss;
    ss 
        << "object corresponding to identifier " 
        << identifier
        << " is neither commit or tree"; 
    
    throw std::logic_error(ss.str());
}

const char *colors[] = {
	"\033[m", /* reset */
	"\033[1m", /* bold */
	"\033[31m", /* red */
	"\033[32m", /* green */
	"\033[36m" /* cyan */
};

static int printer(
	const git_diff_delta *,
	const git_diff_range *,
	char usage,
	const char *line,
	size_t,
	int & last_color)
{
	int color = 0;

	if (last_color >= 0) {
		switch (usage) {
		case GIT_DIFF_LINE_ADDITION: color = 3; break;
		case GIT_DIFF_LINE_DELETION: color = 2; break;
		case GIT_DIFF_LINE_ADD_EOFNL: color = 3; break;
		case GIT_DIFF_LINE_DEL_EOFNL: color = 2; break;
		case GIT_DIFF_LINE_FILE_HDR: color = 1; break;
		case GIT_DIFF_LINE_HUNK_HDR: color = 4; break;
		default: color = 0;
		}
		if (color != last_color) {
			if (last_color == 1 || color == 1)
				fputs(colors[0], stdout);
			fputs(colors[color], stdout);
			last_color = color;
		}
	}

	fputs(line, stdout);
	return 0;
}

static int check_uint16_param(const char *arg, const char *pattern, uint16_t *val)
{
	size_t len = strlen(pattern);
	uint16_t strval;
	char *endptr = NULL;
	if (strncmp(arg, pattern, len))
		return 0;
	if (arg[len] == '\0' && pattern[len - 1] != '=')
		return 1;
	if (arg[len] == '=')
		len++;
	strval = strtoul(arg + len, &endptr, 0);
	if (endptr == arg)
		return 0;
	*val = strval;
	return 1;
}

static int check_str_param(const char *arg, const char *pattern, const char **val)
{
	size_t len = strlen(pattern);
	if (strncmp(arg, pattern, len))
		return 0;
	*val = (const char *)(arg + len);
	return 1;
}

static void usage(const char *message, const char *arg)
{
	if (message && arg)
		fprintf(stderr, "%s: %s\n", message, arg);
	else if (message)
		fprintf(stderr, "%s\n", message);
	fprintf(stderr, "usage: diff [<tree-oid> [<tree-oid>]]\n");
	exit(1);
}

enum {
	FORMAT_PATCH = 0,
	FORMAT_COMPACT = 1,
	FORMAT_RAW = 2
};

/* <sha1> <sha2> */
/* <sha1> --cached */
/* <sha1> */
/* --cached */
/* nothing */

DiffList diff(Repository & repo, Tree & t1, Tree & t2, bool cached, git_diff_options const & opts)
{
    auto r_ptr = repo.ptr();

	if (t1 && t2)
        return diff(r_ptr, t1, t2, opts);
	else if (t1 && cached)
		return diff_to_index(r_ptr, t1, opts);
	else if (t1) 
        return std::move(diff_to_index(r_ptr, t1, opts).merge(diff_index_to_workdir(r_ptr, opts)));
	else if (cached) 
    {
        auto tree = resolve_to_tree(repo, "HEAD");
		return diff_to_index(r_ptr, tree, opts);
    }
	else
		return diff_index_to_workdir(r_ptr, opts);
}

int main(int argc, char *argv[])
{
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff_find_options findopts = GIT_DIFF_FIND_OPTIONS_INIT;
	int i, color = -1, format = FORMAT_PATCH, cached = 0;
	char *a, *treeish1 = NULL, *treeish2 = NULL;
	const char *dir = ".";

    git::ThreadsInitializer threads_initializer;

	/* parse arguments as copied from git-diff */

	for (i = 1; i < argc; ++i) {
		a = argv[i];

		if (a[0] != '-') {
			if (treeish1 == NULL)
				treeish1 = a;
			else if (treeish2 == NULL)
				treeish2 = a;
			else
				usage("Only one or two tree identifiers can be provided", NULL);
		}
		else if (!strcmp(a, "-p") || !strcmp(a, "-u") ||
			!strcmp(a, "--patch"))
			format = FORMAT_PATCH;
		else if (!strcmp(a, "--cached"))
			cached = 1;
		else if (!strcmp(a, "--name-status"))
			format = FORMAT_COMPACT;
		else if (!strcmp(a, "--raw"))
			format = FORMAT_RAW;
		else if (!strcmp(a, "--color"))
			color = 0;
		else if (!strcmp(a, "--no-color"))
			color = -1;
		else if (!strcmp(a, "-R"))
			opts.flags |= GIT_DIFF_REVERSE;
		else if (!strcmp(a, "-a") || !strcmp(a, "--text"))
			opts.flags |= GIT_DIFF_FORCE_TEXT;
		else if (!strcmp(a, "--ignore-space-at-eol"))
			opts.flags |= GIT_DIFF_IGNORE_WHITESPACE_EOL;
		else if (!strcmp(a, "-b") || !strcmp(a, "--ignore-space-change"))
			opts.flags |= GIT_DIFF_IGNORE_WHITESPACE_CHANGE;
		else if (!strcmp(a, "-w") || !strcmp(a, "--ignore-all-space"))
			opts.flags |= GIT_DIFF_IGNORE_WHITESPACE;
		else if (!strcmp(a, "--ignored"))
			opts.flags |= GIT_DIFF_INCLUDE_IGNORED;
		else if (!strcmp(a, "--untracked"))
			opts.flags |= GIT_DIFF_INCLUDE_UNTRACKED;
		else if (check_uint16_param(a, "-M", &findopts.rename_threshold) ||
				 check_uint16_param(a, "--find-renames",
					&findopts.rename_threshold))
			findopts.flags |= GIT_DIFF_FIND_RENAMES;
		else if (check_uint16_param(a, "-C", &findopts.copy_threshold) ||
				 check_uint16_param(a, "--find-copies",
					&findopts.copy_threshold))
			findopts.flags |= GIT_DIFF_FIND_COPIES;
		else if (!strcmp(a, "--find-copies-harder"))
			findopts.flags |= GIT_DIFF_FIND_COPIES_FROM_UNMODIFIED;
		else if (!strncmp(a, "-B", 2) || !strncmp(a, "--break-rewrites", 16)) {
			/* TODO: parse thresholds */
			findopts.flags |= GIT_DIFF_FIND_REWRITES;
		}
		else if (!check_uint16_param(a, "-U", &opts.context_lines) &&
			!check_uint16_param(a, "--unified=", &opts.context_lines) &&
			!check_uint16_param(a, "--inter-hunk-context=",
				&opts.interhunk_lines) &&
			!check_str_param(a, "--src-prefix=", &opts.old_prefix) &&
			!check_str_param(a, "--dst-prefix=", &opts.new_prefix) &&
			!check_str_param(a, "--git-dir=", &dir))
			usage("Unknown arg", a);
	}

	git::Repository repo(dir);

    git::Tree t1, t2;

	if (treeish1)
		t1 = resolve_to_tree(repo, treeish1);
	if (treeish2)
        t2 = resolve_to_tree(repo, treeish2);

    DiffList diff_list = diff(repo, t1, t2, cached, opts);                

	if ((findopts.flags & GIT_DIFF_FIND_ALL) != 0)
		diff_list.find_similar(findopts);

	if (color >= 0)
		fputs(colors[0], stdout);

    using namespace std::placeholders;
    auto print_cb = std::bind(&printer, _1, _2, _3, _4, _5, std::ref(color));

	switch (format) {
	case FORMAT_PATCH:
		diff_list.print_patch(print_cb);
		break;
	case FORMAT_COMPACT:
		diff_list.print_compact(print_cb);
		break;
	case FORMAT_RAW:
		diff_list.print_raw(print_cb);
		break;
	}

	if (color >= 0)
		fputs(colors[0], stdout);

	return 0;
}

