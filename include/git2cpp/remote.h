#pragma once

struct git_remote;

namespace git
{
    struct Remote
    {
        const char * url() const;

    private:
        friend struct Repository;

        explicit Remote(git_remote * remote)
            : remote_(remote)
        {}

        git_remote * remote_;
    };
}
