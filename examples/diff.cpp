#include <git2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "git2cpp/diff.h"
#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"

using namespace git;

Tree resolve_to_tree(Repository const & repo, const char * identifier)
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

const char * colors[] = {
    "\033[m",   /* reset */
    "\033[1m",  /* bold */
    "\033[31m", /* red */
    "\033[32m", /* green */
    "\033[36m"  /* cyan */
};

static void usage(const char * message, const char * arg)
{
    if (message && arg)
        fprintf(stderr, "%s: %s\n", message, arg);
    else if (message)
        fprintf(stderr, "%s\n", message);
    fprintf(stderr, "usage: diff [<tree-oid> [<tree-oid>]]\n");
    exit(1);
}

namespace output
{
    typedef tagged_mask_t<struct output_tag> type;

    const type diff(1 << 0);
    const type stat(1 << 1);
    const type shortstat(1 << 2);
    const type numstat(1 << 3);
    const type summary(1 << 4);
}

enum class cache_t
{
    normal,
    only,
    none
};

/** The 'opts' struct captures all the various parsed command line options. */
struct opts_t
{
    git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;
    git_diff_find_options findopts = GIT_DIFF_FIND_OPTIONS_INIT;
    int color = -1;
    cache_t cache = cache_t::normal;
    output::type output;
    diff::format format = diff::format::patch;
    const char * treeish1 = nullptr;
    const char * treeish2 = nullptr;
    const char * dir = ".";

    opts_t(int argc, char * argv[]);
};

size_t is_prefixed(const char * str, const char * pfx)
{
    size_t len = strlen(pfx);
    return strncmp(str, pfx, len) ? 0 : len;
}

void fatal(const char * message, const char * extra)
{
    if (extra)
        fprintf(stderr, "%s %s\n", message, extra);
    else
        fprintf(stderr, "%s\n", message);

    exit(1);
}

struct args_info
{
private:
    int argc;
    char ** argv;

public:
    int pos;

    args_info(int argc, char ** argv)
        : argc(argc)
        , argv(argv)
        , pos(0)
    {
    }

    int match_str(const char ** out, const char * opt);
    int match_uint16(uint16_t * out, const char * opt);

private:
    const char * match_numeric(const char * opt);
};

int args_info::match_str(const char ** out, const char * opt)
{
    const char * found = argv[pos];
    size_t len = is_prefixed(found, opt);

    if (!len)
        return 0;

    if (!found[len])
    {
        if (pos + 1 == argc)
            fatal("expected value following argument", opt);
        ++pos;
        *out = argv[pos];
        return 1;
    }

    if (found[len] == '=')
    {
        *out = found + len + 1;
        return 1;
    }

    return 0;
}

const char * args_info::match_numeric(const char * opt)
{
    const char * found = argv[pos];
    size_t len = is_prefixed(found, opt);

    if (!len)
        return NULL;

    if (!found[len])
    {
        if (pos + 1 == argc)
            fatal("expected numeric value following argument", opt);
        pos += 1;
        found = argv[pos];
    }
    else
    {
        found = found + len;
        if (*found == '=')
            found++;
    }

    return found;
}

int args_info::match_uint16(uint16_t * out, const char * opt)
{
    const char * found = match_numeric(opt);
    uint16_t val;
    char * endptr = NULL;

    if (!found)
        return 0;

    val = (uint16_t)strtoul(found, &endptr, 0);
    if (!endptr || *endptr != '\0')
        fatal("expected number after argument", opt);

    if (out)
        *out = val;
    return 1;
}

int match_uint16_arg(uint32_t * out, args_info & args, const char * opt)
{
    uint16_t tmp;
    const int res = args.match_uint16(&tmp, opt);
    *out = tmp;
    return res;
}

/** Parse arguments as copied from git-diff. */
opts_t::opts_t(int argc, char * argv[])
{
    args_info args(argc, argv);

    for (args.pos = 1; args.pos < argc; ++args.pos)
    {
        const char * a = argv[args.pos];

        if (a[0] != '-')
        {
            if (treeish1 == NULL)
                treeish1 = a;
            else if (treeish2 == NULL)
                treeish2 = a;
            else
                usage("Only one or two tree identifiers can be provided", NULL);
        }
        else if (!strcmp(a, "-p") || !strcmp(a, "-u") ||
                 !strcmp(a, "--patch"))
        {
            output |= output::diff;
            format = diff::format::patch;
        }
        else if (!strcmp(a, "--cached"))
            cache = cache_t::only;
        else if (!strcmp(a, "--nocache"))
            cache = cache_t::none;
        else if (!strcmp(a, "--name-only") || !strcmp(a, "--format=name"))
            format = diff::format::name_only;
        else if (!strcmp(a, "--name-status") ||
                 !strcmp(a, "--format=name-status"))
            format = diff::format::name_status;
        else if (!strcmp(a, "--raw") || !strcmp(a, "--format=raw"))
            format = diff::format::raw;
        else if (!strcmp(a, "--format=diff-index"))
        {
            format = diff::format::raw;
            diffopts.id_abbrev = 40;
        }
        else if (!strcmp(a, "--color"))
            color = 0;
        else if (!strcmp(a, "--no-color"))
            color = -1;
        else if (!strcmp(a, "-R"))
            diffopts.flags |= GIT_DIFF_REVERSE;
        else if (!strcmp(a, "-a") || !strcmp(a, "--text"))
            diffopts.flags |= GIT_DIFF_FORCE_TEXT;
        else if (!strcmp(a, "--ignore-space-at-eol"))
            diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE_EOL;
        else if (!strcmp(a, "-b") || !strcmp(a, "--ignore-space-change"))
            diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE_CHANGE;
        else if (!strcmp(a, "-w") || !strcmp(a, "--ignore-all-space"))
            diffopts.flags |= GIT_DIFF_IGNORE_WHITESPACE;
        else if (!strcmp(a, "--ignored"))
            diffopts.flags |= GIT_DIFF_INCLUDE_IGNORED;
        else if (!strcmp(a, "--untracked"))
            diffopts.flags |= GIT_DIFF_INCLUDE_UNTRACKED;
        else if (!strcmp(a, "--patience"))
            diffopts.flags |= GIT_DIFF_PATIENCE;
        else if (!strcmp(a, "--minimal"))
            diffopts.flags |= GIT_DIFF_MINIMAL;
        else if (!strcmp(a, "--stat"))
            output |= output::stat;
        else if (!strcmp(a, "--numstat"))
            output |= output::numstat;
        else if (!strcmp(a, "--shortstat"))
            output |= output::shortstat;
        else if (!strcmp(a, "--summary"))
            output |= output::summary;
        else if (args.match_uint16(
                     &findopts.rename_threshold, "-M") ||
                 args.match_uint16(
                     &findopts.rename_threshold, "--find-renames"))
            findopts.flags |= GIT_DIFF_FIND_RENAMES;
        else if (args.match_uint16(
                     &findopts.copy_threshold, "-C") ||
                 args.match_uint16(
                     &findopts.copy_threshold, "--find-copies"))
            findopts.flags |= GIT_DIFF_FIND_COPIES;
        else if (!strcmp(a, "--find-copies-harder"))
            findopts.flags |= GIT_DIFF_FIND_COPIES_FROM_UNMODIFIED;
        else if (is_prefixed(a, "-B") || is_prefixed(a, "--break-rewrites"))
            /* TODO: parse thresholds */
            findopts.flags |= GIT_DIFF_FIND_REWRITES;
        else if (!match_uint16_arg(
                     &diffopts.context_lines, args, "-U") &&
                 !match_uint16_arg(
                     &diffopts.context_lines, args, "--unified") &&
                 !match_uint16_arg(
                     &diffopts.interhunk_lines, args, "--inter-hunk-context") &&
                 !args.match_uint16(
                     &diffopts.id_abbrev, "--abbrev") &&
                 !args.match_str(&diffopts.old_prefix, "--src-prefix") &&
                 !args.match_str(&diffopts.new_prefix, "--dst-prefix") &&
                 !args.match_str(&dir, "--git-dir"))
            usage("Unknown command line argument", a);
    }
}

Diff find_diff(Repository const & repo, Tree & t1, Tree & t2, cache_t cache, git_diff_options const & opts)
{
    if (t1 && t2)
    {
        return repo.diff(t1, t2, opts);
    }
    else if (cache == cache_t::normal)
    {
        if (t1)
            return repo.diff_to_workdir_with_index(t1, opts);
        else
            return repo.diff_index_to_workdir(opts);
    }
    else
    {
        if (!t1)
            t1 = resolve_to_tree(repo, "HEAD");

        if (cache == cache_t::none)
            return repo.diff_to_workdir(t1, opts);
        else
            return repo.diff_to_index(t1, opts);
    }
}

int color_printer(git_diff_delta const &, git_diff_hunk const &, git_diff_line const & l, int & last_color)
{
    int color = 0;

    if (last_color >= 0)
    {
        switch (l.origin)
        {
        case GIT_DIFF_LINE_ADDITION:
            color = 3;
            break;
        case GIT_DIFF_LINE_DELETION:
            color = 2;
            break;
        case GIT_DIFF_LINE_ADD_EOFNL:
            color = 3;
            break;
        case GIT_DIFF_LINE_DEL_EOFNL:
            color = 2;
            break;
        case GIT_DIFF_LINE_FILE_HDR:
            color = 1;
            break;
        case GIT_DIFF_LINE_HUNK_HDR:
            color = 4;
            break;
        default:
            break;
        }

        if (color != last_color)
        {
            if (last_color == 1 || color == 1)
                fputs(colors[0], stdout);
            fputs(colors[color], stdout);
            last_color = color;
        }
    }

    if (l.origin == GIT_DIFF_LINE_CONTEXT ||
        l.origin == GIT_DIFF_LINE_ADDITION ||
        l.origin == GIT_DIFF_LINE_DELETION)
        fputc(l.origin, stdout);

    fwrite(l.content, 1, l.content_len, stdout);

    return 0;
}

void diff_print_stats(Diff const & diff, output::type output)
{
    namespace fmt = diff::stats::format;
    fmt::type format = fmt::none;
    if (output & output::stat)
        format |= fmt::full;
    if (output & output::shortstat)
        format |= fmt::_short;
    if (output & output::numstat)
        format |= fmt::number;
    if (output & output::summary)
        format |= fmt::include_summary;

    auto stats = diff.stats();
    if (Buffer buf = stats.to_buf(format, 80))
        fputs(buf.ptr(), stdout);
}

int main(int argc, char * argv[])
{
    auto_git_initializer;
    opts_t opts(argc, argv);
    Repository repo(opts.dir);
    Tree t1, t2;
    if (opts.treeish1)
        t1 = resolve_to_tree(repo, opts.treeish1);
    if (opts.treeish2)
        t2 = resolve_to_tree(repo, opts.treeish2);

    Diff diff = find_diff(repo, t1, t2, opts.cache, opts.diffopts);

    if (opts.findopts.flags & GIT_DIFF_FIND_ALL)
        diff.find_similar(opts.findopts);

    if (!opts.output)
        opts.output = output::diff;

    if (opts.output != output::diff)
        diff_print_stats(diff, opts.output);

    if (opts.output & output::diff)
    {
        if (opts.color >= 0)
            fputs(colors[0], stdout);

        using namespace std::placeholders;
        diff.print(opts.format, std::bind(&color_printer, _1, _2, _3, std::ref(opts.color)));

        if (opts.color >= 0)
            fputs(colors[0], stdout);
    }
}
