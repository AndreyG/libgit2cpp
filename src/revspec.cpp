#include "git2cpp/revspec.h"

namespace git
{
    Revspec::Revspec(git_object * single, Repository const & repo)
        : flags_(GIT_REVPARSE_SINGLE) 
        , revspec_(single, repo)
    {}

    Revspec::Revspec(git_revspec const & revspec, Repository const & repo)
        : flags_(revspec.flags)
        , revspec_((revspec.flags & GIT_REVPARSE_SINGLE) 
                        ? Range(revspec.from, repo)
                        : Range(revspec, repo))
    {}

    Object * Revspec::single()
    {
        if (flags_ & GIT_REVPARSE_SINGLE)
            return &revspec_.from;
        else
            return nullptr;
    }

    Revspec::Range const * Revspec::range() const
    {
        if (flags_ & GIT_REVPARSE_SINGLE)
            return nullptr;
        else
            return &revspec_;
    }

    unsigned int Revspec::flags() const
    {
        return flags_;
    }
}

