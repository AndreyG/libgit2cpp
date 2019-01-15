#pragma once

#include <git2/types.h>

struct git_reference;
struct git_oid;

namespace git
{
    struct Reference
    {
        Reference()
            : ref_(nullptr)
        {}

        explicit Reference(git_reference * ref);
        ~Reference();

        explicit operator bool() const { return ref_ != nullptr; }

        const char * name() const;
        git_ref_t type() const;
        git_oid const & target() const;
        const char * symbolic_target() const;

        Reference & operator=(Reference const &) = delete;
        Reference(Reference const &) = delete;

        Reference(Reference &&) noexcept;
        Reference & operator=(Reference && other) noexcept;

    private:
        friend struct Repository;
        git_reference * ptr() const { return ref_; }

    private:
        git_reference * ref_;
    };
}
