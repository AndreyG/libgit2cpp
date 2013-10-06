#include <git2/tag.h>

#include "git2cpp/tag.h"
#include "git2cpp/error.h"

namespace git
{
    Tag::Tag(git_oid const * oid, git_repository * repo)
    {
        if (git_tag_lookup(&tag_, repo, oid))
            throw tag_lookup_error(oid);
    }

    Tag::~Tag()
    {
        git_tag_free(tag_);
    }

    Object Tag::target() const
    {
        git_object * obj;
        git_tag_target(&obj, tag_);
        return Object(obj);
    }

    git_otype Tag::target_type() const
    {
        return git_tag_target_type(tag_);
    }

    const char * Tag::name() const
    {
        return git_tag_name(tag_);
    }

    const char * Tag::message() const
    {
        return git_tag_message(tag_);
    }
}
