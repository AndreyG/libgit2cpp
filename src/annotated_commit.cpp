#include "git2cpp/annotated_commit.h"

#include <git2/annotated_commit.h>

namespace git
{
    git_oid const& AnnotatedCommit::commit_id() const
    {
        return *git_annotated_commit_id(ptr());
    }

    char const* AnnotatedCommit::commit_ref() const
    {
        return git_annotated_commit_ref(ptr());
    }

    void AnnotatedCommit::Destroy::operator()(git_annotated_commit* commit) const
    {
        git_annotated_commit_free(commit);
    }
}
