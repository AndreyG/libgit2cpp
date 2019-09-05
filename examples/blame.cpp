/*
 * libgit2 "blame" example - shows how to use the blame API
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
#include "git2/blame.h"

#ifdef _MSC_VER
#define snprintf sprintf_s
#define strcasecmp strcmpi
#endif

/**
 * This example demonstrates how to invoke the libgit2 blame API to roughly
 * simulate the output of `git blame` and a few of its command line arguments.
 */

struct opts {
    const char * path;
    const char * commitspec = nullptr;
    int start_line;
    int end_line;
    bool C = false;
    bool M = false;
    bool F = false;

    opts(int argc, char *argv[]);
};

int main(int argc, char *argv[])
{
    opts o(argc, argv);
    git_blame_options blameopts = GIT_BLAME_OPTIONS_INIT;

    if (o.M) blameopts.flags |= GIT_BLAME_TRACK_COPIES_SAME_COMMIT_MOVES;
    if (o.C) blameopts.flags |= GIT_BLAME_TRACK_COPIES_SAME_COMMIT_COPIES;
    if (o.F) blameopts.flags |= GIT_BLAME_FIRST_PARENT;

    auto_git_initializer;

    git::Repository repo(".");
    /**
     * The commit range comes in "commitish" form. Use the rev-parse API to
     * nail down the end points.
     */
    if (o.commitspec)
    {
        auto revspec = repo.revparse(o.commitspec);

        if (revspec.flags() & GIT_REVPARSE_SINGLE)
        {
            blameopts.newest_commit = revspec.single()->id();
        }
        else
        {
            auto const & range = *revspec.range();
            blameopts.oldest_commit = range.from.id();
            blameopts.newest_commit = range.to.id();
        }
    }

    /** Run the blame. */
    auto blame = repo.blame_file(o.path, blameopts);

    /**
     * Get the raw data inside the blob for output. We use the
     * `commitish:path/to/file.txt` format to find it.
     */
    std::string spec = git_oid_iszero(&blameopts.newest_commit)
            ? "HEAD"
            : git::id_to_str(blameopts.newest_commit);
    spec += ":";
    spec += o.path;

    auto blob = repo.blob_lookup(repo.revparse_single(spec.c_str()).single()->id());

    char const * rawdata = reinterpret_cast<char const *>(blob.content());
    size_t rawsize = blob.size();

    /** Produce the output. */
    size_t line = 1;
    bool break_on_null_hunk = false;
    for (size_t i = 0; i < rawsize; ++line) {
        const git_blame_hunk *hunk = blame.hunk_byline(line);

        if (break_on_null_hunk && !hunk)
            break;

        char const * eol = reinterpret_cast<char const *>(memchr(rawdata + i, '\n', rawsize - i));
        if (hunk) {
            break_on_null_hunk = true;

            char oid[10] = {0};
            git_oid_tostr(oid, 10, &hunk->final_commit_id);
            char sig[128] = {0};
            snprintf(sig, 127, "%s <%s>", hunk->final_signature->name, hunk->final_signature->email);

            printf("%s ( %-30s %3d) %.*s\n",
                    oid,
                    sig,
                    (int)line,
                    (int)(eol - rawdata - i),
                    rawdata + i);
        }

        i = eol - rawdata + 1;
    }

    return 0;
}

/** Tell the user how to make this thing work. */
static void usage(const char *msg)
{
    if (msg)
        fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "usage: blame [options] [<commit range>] <path>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   <commit range>      example: `HEAD~10..HEAD`, or `1234abcd`\n");
    fprintf(stderr, "   -L <n,m>            process only line range n-m, counting from 1\n");
    fprintf(stderr, "   -M                  find line moves within and across files\n");
    fprintf(stderr, "   -C                  find line copies within and across files\n");
    fprintf(stderr, "   -F                  follow only the first parent commits\n");
    fprintf(stderr, "\n");
    exit(1);
}

/** Parse the arguments. */
opts::opts(int argc, char *argv[])
{
    int i;
    const char * bare_args[3] = {};

    if (argc < 2) usage(nullptr);

    for (i=1; i<argc; i++) {
        char *a = argv[i];

        if (a[0] != '-') {
            int i=0;
            while (bare_args[i] && i < 3) ++i;
            if (i >= 3)
                usage("Invalid argument set");
            bare_args[i] = a;
        }
        else if (!strcmp(a, "--"))
            continue;
        else if (!strcasecmp(a, "-M"))
            M = true;
        else if (!strcasecmp(a, "-C"))
            C = true;
        else if (!strcasecmp(a, "-F"))
            F = true;
        else if (!strcasecmp(a, "-L")) {
            i++; a = argv[i];
            if (i >= argc) throw std::runtime_error("Not enough arguments to -L");
            if (sscanf(a, "%d,%d", &start_line, &end_line) != 2)
            {
                fprintf(stderr, "-L format error\n");
                std::abort();
            }
        }
        else {
            /* commit range */
            if (commitspec) throw std::runtime_error("Only one commit spec allowed");
            commitspec = a;
        }
    }

    /* Handle the bare arguments */
    if (!bare_args[0]) usage("Please specify a path");
    path = bare_args[0];
    if (bare_args[1]) {
        /* <commitspec> <path> */
        path = bare_args[1];
        commitspec = bare_args[0];
    }
    if (bare_args[2]) {
        /* <oldcommit> <newcommit> <path> */
        char spec[128] = {0};
        path = bare_args[2];
        sprintf(spec, "%s..%s", bare_args[0], bare_args[1]);
        commitspec = spec;
    }
}
