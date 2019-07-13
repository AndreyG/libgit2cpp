#pragma once

#include <memory>

struct git_blame;
struct git_blame_hunk;

namespace git
{
    struct Blame
    {
        explicit Blame(git_blame * blame)
            : blame_(blame)
        {}

        git_blame_hunk const * get_hunk_byline(size_t lineno) const;

    private:
        struct Destroy { void operator() (git_blame *) const; };

        std::unique_ptr<git_blame, Destroy> blame_;        
    };
}
