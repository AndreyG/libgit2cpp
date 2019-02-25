#pragma once

#include <git2/types.h>
#include <memory>

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
        Object() = default;
        Object(git_object * obj, Repository const &);

        explicit operator bool() const { return obj_.operator bool(); }

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

    private:
        struct Destroy { void operator() (git_object*) const; };
        std::unique_ptr<git_object, Destroy> obj_;
        Repository const * repo_ = nullptr;
    };
}
