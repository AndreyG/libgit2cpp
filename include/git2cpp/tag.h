#pragma once

#include "object.h"

struct git_tag;

namespace git
{
    struct Tag
    {
        Tag(git_oid const * oid, git_repository * repo);
        ~Tag();

        Tag& operator = (Tag const &) = delete;
        Tag             (Tag const &) = delete;

        Tag(Tag &&);

        Object          target()        const;
        git_otype       target_type()   const;
        const char *    name()          const;
        const char *    message()       const;

    private:
        git_tag * tag_;
    };
}
