#include "git2cpp/object.h"
#include "git2cpp/blob.h"
#include "git2cpp/commit.h"
#include "git2cpp/tree.h"
#include "git2cpp/tag.h"

#include <cassert>

#include <git2/object.h>

namespace git
{
    Object::Object(git_object * obj, Repository const & repo)
        : obj_(obj)
        , repo_(&repo)
    {
    }

    void Object::Destroy::operator()(git_object* obj) const
    {
       git_object_free(obj);
    }

    git_otype Object::type() const
    {
        return git_object_type(obj_.get());
    }

    git_oid const & Object::id() const
    {
        return *git_object_id(obj_.get());
    }

#define DEFINE_METHOD_AS(type_name, enum_element)               \
    git_##type_name * Object::as_##type_name()                  \
    {                                                           \
        assert(type() == GIT_OBJ_##enum_element);               \
        return reinterpret_cast<git_##type_name *>(obj_.get()); \
    }

    DEFINE_METHOD_AS(blob, BLOB)
    DEFINE_METHOD_AS(commit, COMMIT)
    DEFINE_METHOD_AS(tree, TREE)
    DEFINE_METHOD_AS(tag, TAG)

#undef DEFINE_METHOD_AS

    Tree Object::to_tree() /*&&*/
    {
        Tree res(as_tree(), *repo_);
        obj_ = nullptr;
        return res;
    }

    Commit Object::to_commit() /*&&*/
    {
        Commit res(as_commit(), *repo_);
        obj_ = nullptr;
        return res;
    }

    Blob Object::to_blob() /*&&*/
    {
        Blob res(as_blob());
        obj_ = nullptr;
        return res;
    }

    Tag Object::to_tag() /*&&*/
    {
        Tag res(as_tag());
        obj_ = nullptr;
        return res;
    }
}
