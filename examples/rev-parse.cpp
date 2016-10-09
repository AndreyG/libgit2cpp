#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>

#include "git2cpp/initializer.h"
#include "git2cpp/repo.h"
#include "git2cpp/internal/optional.h"

using namespace git;

static void usage(const char *message, const char *arg)
{
	if (message && arg)
		fprintf(stderr, "%s: %s\n", message, arg);
	else if (message)
		fprintf(stderr, "%s\n", message);
	fprintf(stderr, "usage: rev-parse [ --option ] <args>...\n");
	exit(1);
}

struct parse_state {
    internal::optional<Repository> repo;
    std::string repodir = ".";
};

void parse_revision(parse_state & ps, const char *revstr)
{
    if (!ps.repo)
        internal::emplace(ps.repo, ps.repodir);

    Revspec rs = ps.repo->revparse(revstr);

    if (auto commit = rs.single())
    {
		std::cout << id_to_str(commit->id()) << std::endl;
	}
	else 
    {
        auto const & range = *rs.range();
        std::cout << id_to_str(range.to.id()) << std::endl;

		if ((rs.flags() & GIT_REVPARSE_MERGE_BASE) != 0) {
			git_oid base = ps.repo->merge_base(range);
            std::cout << id_to_str(base) << std::endl;
		}

        std::cout << "^" << id_to_str(range.from.id()) << std::endl;
	}
}

int main(int argc, char *argv[])
{
    Initializer threads_initalizer;

	parse_state ps;

	for (int i = 1; i < argc; ++i) {
		const char * a = argv[i];

		if (a[0] != '-') 
			parse_revision(ps, a);
        else if (!strncmp(a, "--git-dir=", strlen("--git-dir=")))
			ps.repodir = a + strlen("--git-dir=");
		else
			usage("Cannot handle argument", a);
	}

	return 0;
}
