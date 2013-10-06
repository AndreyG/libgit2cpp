#pragma once

#include "tree.h"

struct git_commit;
struct git_oid;
struct git_repository;

namespace git
{
    struct Commit
    {
        git_commit const * ptr() const { return commit_; }

        size_t parents_num() const;
        Commit parent(size_t i) const;
        git_oid const * parent_id(size_t i) const;

        Tree tree() const;

        git_repository * owner() const;

        explicit operator bool () const { return commit_; }

        git_oid const * id() const;

        git_signature const * author()      const;
        git_signature const * commiter()    const;

        const char *    message()   const;
        git_time_t      time()      const;

        Commit(git_oid const * oid, git_repository * repo);

        explicit Commit(git_commit * commit = nullptr)
            : commit_(commit)
        {}

        Commit(Commit && other)
            : commit_(other.commit_)
        {
            other.commit_ = nullptr;
        }

        Commit              (Commit const &) = delete;
        Commit& operator =  (Commit const &) = delete; 

        ~Commit();

    private:
        git_commit * commit_;
    };
}

