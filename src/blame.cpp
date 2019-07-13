#include "git2cpp/blame.h"
#include "git2/blame.h"

namespace git
{
    uint32_t Blame::hunk_count() const
    {
        return git_blame_get_hunk_count(blame_.get());
    }

    const git_blame_hunk* Blame::hunk_byindex(uint32_t index) const
    {
        return git_blame_get_hunk_byindex(blame_.get(), index);
    }

    const git_blame_hunk* Blame::hunk_byline(size_t lineno) const
    {
        return git_blame_get_hunk_byline(blame_.get(), lineno);
    }

    void Blame::Destroy::operator() (git_blame * blame) const
    {
        git_blame_free(blame);
    }
}
