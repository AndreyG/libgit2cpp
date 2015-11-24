#include "git2cpp/object.h"

#include <cassert>

extern "C"
{
#include <git2/object.h>
}

namespace git
{
    Object::Object(git_object * obj, Repository const & repo)
        : obj_(obj)
        , repo_(&repo)
    {}

    Object::~Object()
    {
        git_object_free(obj_);
    }

    Object::Object(Object && other)
        : obj_(other.obj_)
        , repo_(other.repo_)
    {
        other.obj_ = nullptr;
    }

    git_otype Object::type() const
    {
        return git_object_type(obj_);
    }

    git_oid const & Object::id() const
    {
        return *git_object_id(obj_);
    }

#define DEFINE_METHOD_AS(type_name, enum_element)                   \
    git_##type_name const * Object::as_##type_name() const          \
    {                                                               \
        assert(type() == GIT_OBJ_##enum_element);                   \
        return reinterpret_cast<git_##type_name const *>(obj_);     \
    }                                                               \

    DEFINE_METHOD_AS(blob,      BLOB)
    DEFINE_METHOD_AS(commit,    COMMIT)
    DEFINE_METHOD_AS(tree,      TREE)
    DEFINE_METHOD_AS(tag,       TAG)

#undef DEFINE_METHOD_AS

    Tree Object::to_tree() /*&&*/
    {
        assert(type() == GIT_OBJ_TREE);
        Tree res(reinterpret_cast<git_tree *>(obj_), *repo_);
        obj_ = nullptr;
        return res;
    }

    Commit Object::to_commit() /*&&*/
    {
        assert(type() == GIT_OBJ_COMMIT);
        Commit res(reinterpret_cast<git_commit *>(obj_), *repo_);
        obj_ = nullptr;
        return res;
    }

    Blob Object::to_blob() /*&&*/
    {
       assert(type() == GIT_OBJ_BLOB);
       Blob res(reinterpret_cast<git_blob *>(obj_));
       obj_ = nullptr;
       return res;
    }
}

