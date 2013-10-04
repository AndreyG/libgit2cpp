#pragma once

struct git_reference;

namespace git
{
    struct Reference
    {
        explicit Reference(git_reference * ref);
        ~Reference();

        const char * name() const;

        Reference& operator =   (Reference const &) = delete;
        Reference               (Reference const &) = delete;

        Reference(Reference &&);

    private:
        git_reference * ref_;
    };
}

