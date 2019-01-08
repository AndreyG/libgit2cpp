#include "git2cpp/tag.h"
#include "git2cpp/object.h"

#include <git2/tag.h>

#include <utility>

namespace git
{
    Tag::Tag(git_tag * tag)
        : tag_(tag)
    {
    }

    Tag::Tag(Tag && other) noexcept
        : tag_(std::exchange(other.tag_, nullptr))
    {
    }

    Tag::~Tag()
    {
        git_tag_free(tag_);
    }

    Object Tag::target(Repository const & repo) const
    {
        git_object * obj;
        git_tag_target(&obj, tag_);
        return Object(obj, repo);
    }

    git_oid const & Tag::target_id() const
    {
        return *git_tag_target_id(tag_);
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

    git_signature const * Tag::tagger() const
    {
        return git_tag_tagger(tag_);
    }
}
