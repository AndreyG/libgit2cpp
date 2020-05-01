#pragma once

#include <memory>

struct git_remote;

namespace git
{
    struct Remote
    {
        const char * url()      const;
        const char * pushurl()  const;

    private:
        friend struct Repository;

        struct Destroy { void operator() (git_remote*) const; };

        explicit Remote(git_remote * remote)
            : remote_(remote)
        {}

        std::unique_ptr<git_remote, Destroy> remote_;
    };
}
