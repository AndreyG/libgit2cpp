#include "git2cpp/blame.h"

#include <git2/blame.h>

namespace git
{
    git_blame_hunk const* Blame::get_hunk_byline(size_t lineno) const
    {
        return git_blame_get_hunk_byline(blame_.get(), lineno);
    }

    void Blame::Destroy::operator()(git_blame* blame) const
    {
        git_blame_free(blame);
    }
}
