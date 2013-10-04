#pragma once

#include "object.h"

extern "C"
{
#include <git2/revparse.h>
}

namespace git
{
    struct Revspec
    {
        struct Range
        {
            Object from, to;

            explicit Range(git_object * single)
                : from  (single)
                , to    (nullptr)
            {}

            explicit Range(git_revspec const & revspec)
                : from  (revspec.from)
                , to    (revspec.to)
            {}
        };

        Object  const * single()    const;
        Range   const * range()     const;

        Object * single();

        unsigned int flags() const;

        Revspec(git_object * single);
        Revspec(git_revspec const &);

    private:
        unsigned int flags_ = 0;
        Range revspec_;
    };
}
