#pragma once

struct git_remote;

namespace git
{
    struct Remote
    {
        const char * url()      const;
        const char * pushurl()  const;

    private:
        friend struct Repository;

        explicit Remote(git_remote * remote)
            : remote_(remote)
        {}

        git_remote * remote_;
    };
}
