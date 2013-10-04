#include "git2cpp/revspec.h"

namespace git
{
    Revspec::Revspec(git_object * single)
        : flags_(GIT_REVPARSE_SINGLE) 
        , revspec_(single)
    {}

    Revspec::Revspec(git_revspec const & revspec)
        : flags_(revspec.flags)
        , revspec_((revspec.flags & GIT_REVPARSE_SINGLE) 
                        ? Range(revspec.from) 
                        : Range(revspec))
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
        if (flags_ & GIT_REVPARSE_SINGLE == 0)
            return &revspec_;
        else
            return nullptr;
    }

    unsigned int Revspec::flags() const
    {
        return flags_;
    }
}

