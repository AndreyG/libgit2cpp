#include "git2cpp/annotated_commit.h"

#include <git2/annotated_commit.h>

#include <utility>

namespace git
{
    AnnotatedCommit::AnnotatedCommit(AnnotatedCommit && other) noexcept
        : commit_(std::exchange(other.commit_, nullptr))
    {
    }

    AnnotatedCommit& AnnotatedCommit::operator=(AnnotatedCommit && other) noexcept
    {
        std::swap(commit_, other.commit_);
        return *this;
    }

    AnnotatedCommit::~AnnotatedCommit()
    {
        git_annotated_commit_free(commit_);
    }

    git_oid const& AnnotatedCommit::commit_id() const
    {
        return *git_annotated_commit_id(commit_);
    }

    char const* AnnotatedCommit::commit_ref() const
    {
        return git_annotated_commit_ref(commit_);
    }
}
