#pragma once

#include <git2/types.h>

struct git_reference;
struct git_oid;

namespace git
{
    struct Reference
    {
        explicit Reference(git_reference * ref);
        ~Reference();

        const char *    name()              const;
        git_ref_t       type()              const;
        git_oid const & target()            const;
        const char *    symbolic_target()   const;

        Reference& operator =   (Reference const &) = delete;
        Reference               (Reference const &) = delete;

        Reference(Reference &&) noexcept;

    private:
        git_reference * ref_;
    };
}

