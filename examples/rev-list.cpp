#include <stdio.h>
#include <string.h>

#include <iostream>

#include "git2cpp/repo.h"
#include "git2cpp/revwalker.h"

extern "C"
{
#include <git2/revwalk.h>
#include <git2/errors.h>
}

using namespace git;

void push_commit(RevWalker const & walk, const git_oid *oid, int hide)
{
	if (hide)
		return walk.hide(oid);
	else
		return walk.push(oid);
}

void push_spec(Repository const & repo, RevWalker const & walk, const char *spec, int hide)
{
	push_commit(walk, revparse_single(repo, spec).id(), hide);
}

void push_range(Repository const & repo, RevWalker const & walk, const char *range, int hide)
{
    auto revspec = repo.revparse(range);

	if (revspec.flags() & GIT_REVPARSE_MERGE_BASE) {
		/* TODO: support "<commit>...<commit>" */
        throw std::runtime_error("unsupported operation");
	}

    auto const & r = *revspec.range();

    push_commit(walk, r.to.id(),  !hide);
    push_commit(walk, r.from.id(), hide);
}

void revwalk_parseopts(Repository const & repo, RevWalker & walk, int nopts, char **opts)
{
	unsigned int sorting = GIT_SORT_NONE;

	int hide = 0;
	for (int i = 0; i < nopts; i++) {
		if (!strcmp(opts[i], "--topo-order")) {
			sorting = GIT_SORT_TOPOLOGICAL | (sorting & GIT_SORT_REVERSE);
			walk.sort(sorting);
		} else if (!strcmp(opts[i], "--date-order")) {
			sorting = GIT_SORT_TIME | (sorting & GIT_SORT_REVERSE);
			walk.sort(sorting);
		} else if (!strcmp(opts[i], "--reverse")) {
			sorting = (sorting & ~GIT_SORT_REVERSE)
			    | ((sorting & GIT_SORT_REVERSE) ? 0 : GIT_SORT_REVERSE);
			walk.sort(sorting);
		} else if (!strcmp(opts[i], "--not")) {
			hide = !hide;
		} else if (opts[i][0] == '^') {
			push_spec(repo, walk, opts[i] + 1, !hide);
		} else if (strstr(opts[i], "..")) {
			push_range(repo, walk, opts[i], hide);
		} else {
		    push_spec(repo, walk, opts[i], hide);
		}
	}
}

int main (int argc, char **argv)
{
    try
    {
        Repository repo(".");
        auto walker = repo.rev_walker();

        revwalk_parseopts(repo, walker, argc-1, argv+1);

        char buf[41];

        while (walker.next(buf)) 
        {
            buf[40] = '\0';
            printf("%s\n", buf);
        }

	    return 0;
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << std::endl;
        if (auto err = giterr_last())
        {
            if (err->message)
                std::cerr << "libgit2 last error: " << err->message << std::endl;
        }
    }
}

