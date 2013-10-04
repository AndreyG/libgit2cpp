#include "git2cpp/reference.h"

extern "C"
{
    #include <git2/refs.h>
}

namespace git
{
    Reference::~Reference()
    {
        git_reference_free(ref_);
    }

    Reference::Reference(git_reference * ref)
        : ref_(ref)
    {}

    const char * Reference::name() const
    {
        return git_reference_name(ref_);
    }
}
