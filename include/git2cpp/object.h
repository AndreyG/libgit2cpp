#pragma once

extern "C"
{
#include <git2/types.h>
}

struct git_object;
struct git_oid;

namespace git
{
    struct Object
    {
        explicit Object(git_object * obj);
        ~Object();

        explicit operator bool() const { return obj_; }

        git_otype       type()  const;
        git_oid const * id()    const;

        git_blob    const * as_blob()   const;
        git_commit  const * as_commit() const;
        git_tree    const * as_tree()   const;
        git_tag     const * as_tag()    const;

        Object              (Object const &) = delete;
        Object& operator =  (Object const &) = delete;

        Object(Object && other);

    private:
        git_object * obj_;
    };
}

