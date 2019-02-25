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

    void Tag::Destroy::operator()(git_tag* tag) const
    {
        git_tag_free(tag);
    }

    Object Tag::target(Repository const & repo) const
    {
        git_object * obj;
        git_tag_target(&obj, tag_.get());
        return Object(obj, repo);
    }

    git_oid const & Tag::target_id() const
    {
        return *git_tag_target_id(tag_.get());
    }

    git_otype Tag::target_type() const
    {
        return git_tag_target_type(tag_.get());
    }

    const char * Tag::name() const
    {
        return git_tag_name(tag_.get());
    }

    const char * Tag::message() const
    {
        return git_tag_message(tag_.get());
    }

    git_signature const * Tag::tagger() const
    {
        return git_tag_tagger(tag_.get());
    }

}
