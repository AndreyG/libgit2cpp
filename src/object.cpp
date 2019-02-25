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
    git_##type_name const * Object::as_##type_name() const      \
    {                                                           \
        assert(type() == GIT_OBJ_##enum_element);               \
        return reinterpret_cast<git_##type_name const *>(obj_.get()); \
    }

    DEFINE_METHOD_AS(blob, BLOB)
    DEFINE_METHOD_AS(commit, COMMIT)
    DEFINE_METHOD_AS(tree, TREE)
    DEFINE_METHOD_AS(tag, TAG)

#undef DEFINE_METHOD_AS

    Tree Object::to_tree() /*&&*/
    {
        assert(type() == GIT_OBJ_TREE);
        Tree res(reinterpret_cast<git_tree *>(obj_.get()), *repo_);
        obj_ = nullptr;
        return res;
    }

    Commit Object::to_commit() /*&&*/
    {
        assert(type() == GIT_OBJ_COMMIT);
        Commit res(reinterpret_cast<git_commit *>(obj_.get()), *repo_);
        obj_ = nullptr;
        return res;
    }

    Blob Object::to_blob() /*&&*/
    {
        assert(type() == GIT_OBJ_BLOB);
        Blob res(reinterpret_cast<git_blob *>(obj_.get()));
        obj_ = nullptr;
        return res;
    }

    Tag Object::to_tag() /*&&*/
    {
        assert(type() == GIT_OBJ_TAG);
        Tag res(reinterpret_cast<git_tag *>(obj_.get()));
        obj_ = nullptr;
        return res;
    }
}
