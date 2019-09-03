/*
 * libgit2 "checkout" example - shows how to perform checkouts
 *
 * Written by the libgit2 contributors
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "git2cpp/repo.h"
#include "git2cpp/initializer.h"
#include "git2cpp/annotated_commit.h"

#include <git2/errors.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* Define the printf format specifer to use for size_t output */
#if defined(_MSC_VER) || defined(__MINGW32__)
#	define PRIuZ "Iu"
#	define PRIxZ "Ix"
#	define PRIdZ "Id"
#else
#	define PRIuZ "zu"
#	define PRIxZ "zx"
#	define PRIdZ "zd"
#endif

/**
 * The following example demonstrates how to do checkouts with libgit2.
 *
 * Recognized options are :
 *  --force: force the checkout to happen.
 *  --[no-]progress: show checkout progress, on by default.
 *  --perf: show performance data.
 */

namespace {

struct args_info {
	int    argc;
	char **argv;
	int    pos;
};

typedef struct {
	int force : 1;
	int progress : 1;
	int perf : 1;
} checkout_options;

[[noreturn]] void print_usage()
{
	fprintf(stderr, "usage: checkout [options] <branch>\n"
		"Options are :\n"
		"  --git-dir: use the following git repository.\n"
		"  --force: force the checkout.\n"
		"  --[no-]progress: show checkout progress.\n"
		"  --perf: show performance data.\n");
	exit(1);
}

bool match_bool_arg(int *out, args_info *args, const char *opt)
{
	const char *found = args->argv[args->pos];

	if (!strcmp(found, opt)) {
		*out = 1;
		return true;
	}

	if (!strncmp(found, "--no-", strlen("--no-")) &&
	    !strcmp(found + strlen("--no-"), opt + 2)) {
		*out = 0;
		return true;
	}

	*out = -1;
	return false;
}

[[noreturn]] void fatal(const char *message, const char *extra)
{
	if (extra)
		fprintf(stderr, "%s %s\n", message, extra);
	else
		fprintf(stderr, "%s\n", message);

	exit(1);
}

size_t is_prefixed(const char *str, const char *pfx)
{
	size_t len = strlen(pfx);
	return strncmp(str, pfx, len) ? 0 : len;
}

bool match_str_arg(const char **out, struct args_info *args, const char *opt)
{
	const char *found = args->argv[args->pos];
	size_t len = is_prefixed(found, opt);

	if (!len)
		return false;

	if (!found[len]) {
		if (args->pos + 1 == args->argc)
			fatal("expected value following argument", opt);
		args->pos += 1;
		*out = args->argv[args->pos];
		return true;
	}

	if (found[len] == '=') {
		*out = found + len + 1;
		return true;
	}

	return false;
}

void parse_options(const char **repo_path, checkout_options *opts, struct args_info *args)
{
	if (args->argc <= 1)
		print_usage();

	memset(opts, 0, sizeof(*opts));

	/* Default values */
	opts->progress = 1;

	for (args->pos = 1; args->pos < args->argc; ++args->pos) {
		const char *curr = args->argv[args->pos];
		int bool_arg;

		if (strcmp(curr, "--") == 0) {
			break;
		} else if (!strcmp(curr, "--force")) {
			opts->force = 1;
		} else if (match_bool_arg(&bool_arg, args, "--progress")) {
			opts->progress = bool_arg;
		} else if (match_bool_arg(&bool_arg, args, "--perf")) {
			opts->perf = bool_arg;
		} else if (match_str_arg(repo_path, args, "--git-dir")) {
			continue;
		} else {
			break;
		}
	}
}

/**
 * This function is called to report progression, ie. it's called once with
 * a NULL path and the number of total steps, then for each subsequent path,
 * the current completed_step value.
 */
void print_checkout_progress(const char *path, size_t completed_steps, size_t total_steps, void */*payload*/)
{
	if (path == NULL) {
		printf("checkout started: %" PRIuZ " steps\n", total_steps);
	} else {
		printf("checkout: %s %" PRIuZ "/%" PRIuZ "\n", path, completed_steps, total_steps);
	}
}

/**
 * This function is called when the checkout completes, and is used to report the
 * number of syscalls performed.
 */
void print_perf_data(const git_checkout_perfdata *perfdata, void* /*payload*/)
{
	printf("perf: stat: %" PRIuZ " mkdir: %" PRIuZ " chmod: %" PRIuZ "\n",
	       perfdata->stat_calls, perfdata->mkdir_calls, perfdata->chmod_calls);
}

/**
 * This is the main "checkout <branch>" function, responsible for performing
 * a branch-based checkout.
 */
void perform_checkout_ref(git::Repository & repo, git::AnnotatedCommit const & target, checkout_options const & opts)
{
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

	/** Setup our checkout options from the parsed options */
	checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	if (opts.force)
		checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	if (opts.progress)
		checkout_opts.progress_cb = print_checkout_progress;

	if (opts.perf)
		checkout_opts.perfdata_cb = print_perf_data;

	/** Grab the commit we're interested to move to */
        auto target_commit = repo.commit_lookup(target.commit_id());
	/**
	 * Perform the checkout so the workdir corresponds to what target_commit
	 * contains.
	 *
	 * Note that it's okay to pass a git_commit here, because it will be 
	 * peeled to a tree.
	 */
	if (repo.checkout_tree(target_commit, checkout_opts))
        {
                fprintf(stderr, "failed to checkout tree: %s\n", git_error_last()->message);
		return;
	}

	/**
	 * Now that the checkout has completed, we have to update HEAD.
	 *
	 * Depending on the "origin" of target (ie. it's an OID or a branch name),
	 * we might need to detach HEAD.
	 */
	int err;
	if (auto ref = target.commit_ref()) {
        err = repo.set_head(ref);
	} else {
		err = repo.set_head_detached(target);
	}
	if (err != 0) {
                fprintf(stderr, "failed to update HEAD reference: %s\n", git_error_last()->message);
	}
}

git::AnnotatedCommit resolve_refish(git::Repository const & repo, const char *refish)
{
    if (auto ref = repo.dwim(refish))
        return repo.annotated_commit_from_ref(ref);

    auto obj = repo.revparse_single(refish);
    return repo.annotated_commit_lookup(obj.single()->id());
}

}

/** That example's entry point */
int main(int argc, char **argv)
{
	args_info args { argc, argv };
	checkout_options opts;
	const char *path = ".";

	/** Parse our command line options */
	parse_options(&path, &opts, &args);

	/** Initialize the library */
	auto_git_initializer;

    git::Repository repo(path);

	/** Make sure we're not about to checkout while something else is going on */
	auto const state = repo.state();
	if (state != GIT_REPOSITORY_STATE_NONE) {
		fprintf(stderr, "repository is in unexpected state %d\n", state);
		return EXIT_FAILURE;
	}

	if (args.pos >= args.argc) {
		fprintf(stderr, "unhandled\n");
		return EXIT_FAILURE;
	}

    if (strcmp("--", args.argv[args.pos]))
    {
		/**
		 * Try to checkout the given path
		 */

		fprintf(stderr, "unhandled path-based checkout\n");
		return EXIT_FAILURE;
	}
    
	/**
	 * Try to resolve a "refish" argument to a target libgit2 can use
	 */
	auto checkout_target = resolve_refish(repo, args.argv[args.pos]);
	perform_checkout_ref(repo, checkout_target, opts);
}
