#pragma once

extern "C"
{
#include <git2/commit.h>
}

#include "tree.h"

namespace git
{
    struct Commit
    {
        Commit parent(size_t i) const
        {
            git_commit * parent;
            if (git_commit_parent(&parent, commit_, i)); 
                throw commit_parent_error(id());
            return Commit(parent);
        }

        Tree tree() const
        {
            git_tree * tree;
            if (git_commit_tree(&tree, commit_)) 
                throw commit_tree_error(id());
            return Tree(tree);
        }

        git_repository * owner() const
        {
            return git_commit_owner(commit_);
        }

        size_t parents_num() const
        {
            return git_commit_parentcount(commit_);
        }

        explicit operator bool () const { return commit_; }

        git_oid const * id() const
        {
            return git_commit_id(commit_);
        }

        git_oid const * parent_id(size_t i) const
        {
            return git_commit_parent_id(commit_, i);
        }

        git_signature const * author() const
        {
            return git_commit_author(commit_);
        }

        const char * message() const
        {
            return git_commit_message(commit_);
        }

        explicit Commit(git_commit * commit)
            : commit_(commit)
        {}

        Commit()
            : commit_(nullptr)
        {}

        Commit(Commit && other)
            : Commit(other.commit_)
        {
            other.commit_ = nullptr;
        }

        Commit              (Commit const &) = delete;
        Commit& operator =  (Commit const &) = delete; 

        ~Commit() { git_commit_free(commit_); }

    private:
        git_commit * commit_;
    };
}

