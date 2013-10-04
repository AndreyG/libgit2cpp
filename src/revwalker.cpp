extern "C"
{
#include <git2/revwalk.h>
}

#include "git2cpp/error.h"
#include "git2cpp/revwalker.h"

namespace git
{
    RevWalker::RevWalker(git_repository * repo)
    {
        if (git_revwalk_new(&walker_, repo))
            throw revwalk_new_error();
    }

    RevWalker::~RevWalker()
    {
        git_revwalk_free(walker_);
    }

    void RevWalker::sort(int sorting)
    {
        git_revwalk_sorting(walker_, sorting);
    }

    void RevWalker::push_head() const
    {
        if (git_revwalk_push_head(walker_))
            throw invalid_head_error();
    }

    void RevWalker::hide(git_oid const * obj) const
    {
        if (git_revwalk_hide(walker_, obj))
            throw non_commit_object_error(obj);
    }

    void RevWalker::push(git_oid const * obj) const
    {
        if (git_revwalk_push(walker_, obj))
            throw non_commit_object_error(obj);
    }

    boost::optional<git_oid> RevWalker::next() const
    {
        git_oid oid;
        if (git_revwalk_next(&oid, walker_) == 0)
            return oid;
        else
            return boost::none;
    }
}

