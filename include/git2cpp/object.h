#pragma once

#include <git2/types.h>

struct git_object;
struct git_oid;
struct git_tree;
struct git_tag;

namespace git
{
    struct Repository;

    struct Blob;
    struct Commit;
    struct Tree;
    struct Tag;

    struct Object
    {
        Object() {}

        Object(git_object * obj, Repository const &);
        ~Object();

        explicit operator bool() const { return obj_ != nullptr; }

        git_otype type() const;
        git_oid const & id() const;

        git_blob const * as_blob() const;
        git_commit const * as_commit() const;
        git_tree const * as_tree() const;
        git_tag const * as_tag() const;

        Commit to_commit() /*&&*/;
        Tree to_tree() /*&&*/;
        Blob to_blob() /*&&*/;
        Tag  to_tag()  /*&&*/;

        Object(Object const &) = delete;
        Object & operator=(Object const &) = delete;

        Object(Object && other) noexcept;

    private:
        git_object * obj_ = nullptr;
        Repository const * repo_ = nullptr;
    };
}
