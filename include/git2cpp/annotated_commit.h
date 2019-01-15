#pragma once

struct git_annotated_commit;
struct git_oid;

namespace git
{
    struct AnnotatedCommit
    {
        AnnotatedCommit(git_annotated_commit * commit)
            : commit_(commit)
        {}

        AnnotatedCommit(AnnotatedCommit const &) = delete;
        AnnotatedCommit& operator = (AnnotatedCommit const &) = delete;

        AnnotatedCommit(AnnotatedCommit &&) noexcept;
        AnnotatedCommit& operator = (AnnotatedCommit&&) noexcept;

        ~AnnotatedCommit();

        git_oid const & commit_id()  const;
        char const *    commit_ref() const;

    private:
        friend struct Repository;
        git_annotated_commit * ptr() const { return commit_; }

    private:
        git_annotated_commit * commit_;
    };
}
