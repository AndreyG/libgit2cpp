/*
 * libgit2 "remote" example - shows how to modify remotes for a repo
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

#include "git2/errors.h"

#include <string.h>

/**
 * This is a sample program that is similar to "git remote".  See the
 * documentation for that (try "git help remote") to understand what this
 * program is emulating.
 *
 * This demonstrates using the libgit2 APIs to modify remotes of a repository.
 */

namespace {

[[noreturn]] void usage(const char *msg, const char *arg = nullptr)
{
    fputs("usage: remote add <name> <url>\n", stderr);
    fputs("       remote remove <name>\n", stderr);
    fputs("       remote rename <old> <new>\n", stderr);
    fputs("       remote set-url [--push] <name> <newurl>\n", stderr);
    fputs("       remote show [-v|--verbose]\n", stderr);

    if (msg && !arg)
        fprintf(stderr, "\n%s\n", msg);
    else if (msg && arg)
        fprintf(stderr, "\n%s: %s\n", msg, arg);
    exit(1);
}

[[noreturn]] void report_error(const char *message)
{
    const git_error *lg2err = git_error_last();
    const char *lg2msg = "", *lg2spacer = "";

    if (lg2err && lg2err->message)
    {
        lg2msg = lg2err->message;
        lg2spacer = " - ";
    }

    fprintf(stderr, "%s %s%s\n", message, lg2spacer, lg2msg);
    exit(1);
}

enum class Subcmd {
    add,
    remove,
    rename,
    seturl,
    show,
};

struct Options {
    Subcmd cmd;

    /* for command-specific args */
    int argc;
    char **argv;

    Options(int argc, char **argv)
        : argc(argc - 2) /* executable and subcommand are removed */
        , argv(argv + 2)
    {
        char *arg = argv[1];
        if (argc < 2)
            usage("no command specified");

        if (!strcmp(arg, "add")) {
            cmd = Subcmd::add;
        } else if (!strcmp(arg, "remove")) {
            cmd = Subcmd::remove;
        } else if (!strcmp(arg, "rename")) {
            cmd = Subcmd::rename;
        } else if (!strcmp(arg, "set-url")) {
            cmd = Subcmd::seturl;
        } else if (!strcmp(arg, "show")) {
            cmd = Subcmd::show;
        } else {
            usage("command is not valid", arg);
        }
    }
};

using git::Repository;

void cmd_add(Repository & repo, Options const & o)
{
    if (o.argc != 2)
        usage("you need to specify a name and URL");

    auto name = o.argv[0], url = o.argv[1];
    repo.create_remote(name, url);
}

void cmd_remove(Repository & repo, Options const & o)
{
    if (o.argc != 1)
        usage("you need to specify a name");

    auto name = o.argv[0];
    repo.delete_remote(name);
}

void cmd_rename(Repository & repo, Options const & o)
{
    if (o.argc != 2)
        usage("you need to specify old and new remote name");

    auto old_name = o.argv[0];
    auto new_name = o.argv[1];
    if (auto problems = repo.rename_remote(old_name, new_name))
    {
        for (size_t i = 0, n = problems->count(); i != n; ++i)
            puts((*problems)[i]);
    }
}

void cmd_seturl(Repository & repo, Options const & o)
{
    bool push = false;
    char const * name = nullptr, *url = nullptr;

    for (auto i = 0; i != o.argc; ++i)
    {
        char const * arg = o.argv[i];

        if (!strcmp(arg, "--push")) {
            push = 1;
        } else if (arg[0] != '-' && !name) {
            name = arg;
        } else if (arg[0] != '-' && !url) {
            url = arg;
        } else {
            usage("invalid argument to set-url", arg);
        }
    }

    if (!name || !url)
        usage("you need to specify remote and the new URL");

    if (push)
        repo.set_pushurl(name, url);
    else
        repo.set_url(name, url);
}

void cmd_show(Repository const & repo, Options const & o)
{
    bool verbose = false;

    for (int i = 0; i != o.argc; ++i)
    {
        auto arg = o.argv[i];

        if (!strcmp(arg, "-v") ||
            !strcmp(arg, "--verbose"))
        {
            verbose = true;
        }
    }

    auto remotes = repo.remotes();

    for (size_t i = 0, n = remotes.count(); i != n; ++i)
    {
        auto name = remotes[i];
        if (!verbose) {
            puts(name);
            continue;
        }

        auto remote = repo.remote(name);

        auto fetch = remote.url();
        if (fetch)
            printf("%s\t%s (fetch)\n", name, fetch);
        auto push = remote.pushurl();
        /* use fetch URL if no distinct push URL has been set */
        push = push ? push : fetch;
        if (push)
            printf("%s\t%s (push)\n", name, push);
    }
}

}

int main(int argc, char *argv[])
{
    Options opt(argc, argv);

    auto_git_initializer;

    auto path_to_repo = git::Repository::discover(".");
    if (!path_to_repo)
        report_error("Could not find repository");

    Repository repo(*path_to_repo);

	switch (opt.cmd)
	{
    case Subcmd::add:
        cmd_add(repo, opt);
        break;
    case Subcmd::remove:
        cmd_remove(repo, opt);
        break;
    case Subcmd::rename:
        cmd_rename(repo, opt);
        break;
    case Subcmd::seturl:
        cmd_seturl(repo, opt);
        break;
    case Subcmd::show:
        cmd_show(repo, opt);
        break;
    }
}
