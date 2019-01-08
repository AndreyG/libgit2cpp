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

#ifdef USE_BOOST
#include <boost/utility/string_view.hpp>

using StringView = boost::string_view;
#else
#include <string_view>

using StringView = std::string_view;
#endif

namespace {

[[noreturn]] void usage(const char * error, const char * arg)
{
    fprintf(stderr, "error: %s '%s'\n", error, arg);
    fprintf(stderr, "usage: init [-q | --quiet] [--bare] "
                    "[--template=<dir>] [--shared[=perms]] <directory>\n");
    exit(1);
}

/* simple string prefix test used in argument parsing */
size_t is_prefixed(StringView arg, StringView pfx)
{
    return arg == pfx ? pfx.size() : 0;
}

/* parse the tail of the --shared= argument */
uint32_t parse_shared(const char * shared)
{
    if (!strcmp(shared, "false") || !strcmp(shared, "umask"))
        return GIT_REPOSITORY_INIT_SHARED_UMASK;

    else if (!strcmp(shared, "true") || !strcmp(shared, "group"))
        return GIT_REPOSITORY_INIT_SHARED_GROUP;

    else if (!strcmp(shared, "all") || !strcmp(shared, "world") ||
             !strcmp(shared, "everybody"))
        return GIT_REPOSITORY_INIT_SHARED_ALL;

    else if (shared[0] == '0')
    {
        char * end = nullptr;
        long val = strtol(shared + 1, &end, 8);
        if (end == shared + 1 || *end != 0)
            usage("invalid octal value for --shared", shared);
        return (uint32_t)val;
    }

    else
        usage("unknown value for --shared", shared);
}

using namespace git;

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

git_repository_init_options make_opts(bool bare, const char * templ,
                                      uint32_t shared,
                                      const char * gitdir,
                                      const char * dir)
{
    git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;

    if (bare)
        opts.flags |= GIT_REPOSITORY_INIT_BARE;

    if (templ)
    {
        opts.flags |= GIT_REPOSITORY_INIT_EXTERNAL_TEMPLATE;
        opts.template_path = templ;
    }

    if (gitdir)
    {
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

}

int main(int argc, char * argv[])
{
    bool no_options = true, quiet = false, bare = false, initial_commit = false;
    uint32_t shared = GIT_REPOSITORY_INIT_SHARED_UMASK;
    const char *templ = nullptr, *gitdir = nullptr, *dir = nullptr;

    auto_git_initializer;

    /* Process arguments */

    for (int i = 1; i < argc; ++i)
    {
        auto arg = argv[i];
        StringView a = arg;

        if (arg[0] == '-')
            no_options = false;

        if (arg[0] != '-')
        {
            if (dir)
                usage("extra argument", arg);
            dir = arg;
        }
        else if (a == "-q" || a == "--quiet")
            quiet = true;
        else if (a == "--bare")
            bare = true;
        else if (auto pfxlen = is_prefixed(a, "--template="))
            templ = arg + pfxlen;
        else if (a == "--separate-git-dir")
            gitdir = argv[++i];
        else if (auto pfxlen = is_prefixed(a, "--separate-git-dir="))
            gitdir = arg + pfxlen;
        else if (a == "--shared")
            shared = GIT_REPOSITORY_INIT_SHARED_GROUP;
        else if (auto pfxlen = is_prefixed(a, "--shared="))
            shared = parse_shared(arg + pfxlen);
        else if (a == "--initial-commit")
            initial_commit = true;
        else
            usage("unknown option", arg);
    }

    if (!dir)
        usage("must specify directory to init", nullptr);

    /* Initialize repository */

    Repository repo = no_options
                          ? Repository(dir, Repository::init)
                          : Repository(dir, Repository::init, make_opts(bare, templ, shared, gitdir, dir));

    /* Print a message to stdout like "git init" does */

    if (!quiet)
    {
        printf("Initialized empty Git repository in %s\n",
               (bare || gitdir) ? repo.path() : repo.workdir());
    }

    /* As an extension to the basic "git init" command, this example
	 * gives the option to create an empty initial commit.  This is
	 * mostly to demonstrate what it takes to do that, but also some
	 * people like to have that empty base commit in their repo.
	 */
    if (initial_commit)
    {
        create_initial_commit(repo);
        printf("Created empty initial commit\n");
    }

    return 0;
}
