#pragma once

#include <git2/types.h>

struct git_tag;
struct git_oid;

namespace git
{
    struct Repository;
    struct Object;

    struct Tag
    {
        explicit Tag(git_tag *);
        ~Tag();

        Tag & operator=(Tag const &) = delete;
        Tag(Tag const &) = delete;

        Tag(Tag &&) noexcept;

        Object target(Repository const &) const;

        git_oid const & target_id()     const;
        git_otype       target_type()   const;

        const char * name() const;
        const char * message() const;
        git_signature const * tagger() const;

    private:
        git_tag * tag_;
    };
}
