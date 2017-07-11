/*
 * This is a sample program that is similar to "git init".  See the
 * documentation for that (try "git help init") to understand what this
 * program is emulating.
 *
 * This demonstrates using the libgit2 APIs to initialize a new repository.
 *
 * This also contains a special additional option that regular "git init"
 * does not support which is "--initial-commit" to make a first empty commit.
 * That is demonstrated in the "create_initial_commit" helper function.
 *
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

static void usage(const char *error, const char *arg)
{
	fprintf(stderr, "error: %s '%s'\n", error, arg);
	fprintf(stderr, "usage: init [-q | --quiet] [--bare] "
			"[--template=<dir>] [--shared[=perms]] <directory>\n");
	exit(1);
}

/* simple string prefix test used in argument parsing */
static size_t is_prefixed(const char *arg, const char *pfx)
{
	size_t len = strlen(pfx);
	return !strncmp(arg, pfx, len) ? len : 0;
}

/* parse the tail of the --shared= argument */
static uint32_t parse_shared(const char *shared)
{
	if (!strcmp(shared, "false") || !strcmp(shared, "umask"))
		return GIT_REPOSITORY_INIT_SHARED_UMASK;

	else if (!strcmp(shared, "true") || !strcmp(shared, "group"))
		return GIT_REPOSITORY_INIT_SHARED_GROUP;

	else if (!strcmp(shared, "all") || !strcmp(shared, "world") ||
			 !strcmp(shared, "everybody"))
		return GIT_REPOSITORY_INIT_SHARED_ALL;

	else if (shared[0] == '0') {
		long val;
		char *end = NULL;
		val = strtol(shared + 1, &end, 8);
		if (end == shared + 1 || *end != 0)
			usage("invalid octal value for --shared", shared);
		return (uint32_t)val;
	}

	else
		usage("unknown value for --shared", shared);

	return 0;
}

using namespace git;

/* forward declaration of helper to make an empty parent-less commit */
static void create_initial_commit(Repository & repo);

git_repository_init_options make_opts(  bool bare, const char * templ, 
                                        uint32_t shared, 
                                        const char * gitdir,
                                        const char * dir  )
{
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;

    if (bare)
        opts.flags |= GIT_REPOSITORY_INIT_BARE;

    if (templ) {
        opts.flags |= GIT_REPOSITORY_INIT_EXTERNAL_TEMPLATE;
        opts.template_path = templ;
    }

    if (gitdir) {
        /* if you specified a separate git directory, then initialize
         * the repository at that path and use the second path as the
         * working directory of the repository (with a git-link file)
         */
        opts.workdir_path = dir;
        dir = gitdir;
    }

    if (shared != 0)
        opts.mode = shared;

    return opts;
}

int main(int argc, char *argv[])
{
	int no_options = 1, quiet = 0, bare = 0, initial_commit = 0, i;
	uint32_t shared = GIT_REPOSITORY_INIT_SHARED_UMASK;
	const char *templ = NULL, *gitdir = NULL, *dir = NULL;
	size_t pfxlen;

    auto_git_initializer;

	/* Process arguments */

	for (i = 1; i < argc; ++i) {
		char *a = argv[i];

		if (a[0] == '-')
			no_options = 0;

		if (a[0] != '-') {
			if (dir != NULL)
				usage("extra argument", a);
			dir = a;
		}
		else if (!strcmp(a, "-q") || !strcmp(a, "--quiet"))
			quiet = 1;
		else if (!strcmp(a, "--bare"))
			bare = 1;
		else if ((pfxlen = is_prefixed(a, "--template=")) > 0)
			templ = a + pfxlen;
		else if (!strcmp(a, "--separate-git-dir"))
			gitdir = argv[++i];
		else if ((pfxlen = is_prefixed(a, "--separate-git-dir=")) > 0)
			gitdir = a + pfxlen;
		else if (!strcmp(a, "--shared"))
			shared = GIT_REPOSITORY_INIT_SHARED_GROUP;
		else if ((pfxlen = is_prefixed(a, "--shared=")) > 0)
			shared = parse_shared(a + pfxlen);
		else if (!strcmp(a, "--initial-commit"))
			initial_commit = 1;
		else
			usage("unknown option", a);
	}

	if (!dir)
		usage("must specify directory to init", NULL);

	/* Initialize repository */

    Repository repo = no_options 
        ? Repository(dir, Repository::init)
        : Repository(dir, Repository::init, make_opts(bare, templ, shared, gitdir, dir));

	/* Print a message to stdout like "git init" does */

	if (!quiet) {
		printf("Initialized empty Git repository in %s\n", 
               (bare || gitdir) ? repo.path() : repo.workdir());
	}

	/* As an extension to the basic "git init" command, this example
	 * gives the option to create an empty initial commit.  This is
	 * mostly to demonstrate what it takes to do that, but also some
	 * people like to have that empty base commit in their repo.
	 */
	if (initial_commit) {
		create_initial_commit(repo);
		printf("Created empty initial commit\n");
	}

	return 0;
}

/* Unlike regular "git init", this example shows how to create an initial
 * empty commit in the repository.  This is the helper function that does
 * that.
 */
static void create_initial_commit(Repository & repo)
{
	/* First use the config to initialize a commit signature for the user */
	Signature sig = repo.signature();

	/* Now let's create an empty tree for this commit */
	Index index = repo.index();

	/* Outside of this example, you could call git_index_add_bypath()
	 * here to put actual files into the index.  For our purposes, we'll
	 * leave it empty for now.
	 */

    git_oid tree_id = index.write_tree();
    Tree tree = repo.tree_lookup(tree_id);

	/* Ready to create the initial commit
	 *
	 * Normally creating a commit would involve looking up the current
	 * HEAD commit and making that be the parent of the initial commit,
	 * but here this is the first commit so there will be no parent.
	 */
    repo.create_commit("HEAD", sig, sig, "Initial commit", tree);
}
