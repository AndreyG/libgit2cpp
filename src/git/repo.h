#pragma once

#include <string>
#include <cassert>
#include <memory>

#include "commit.h"
#include "revwalker.h"

namespace git
{
    struct Repository
    {
        void commit_lookup_c_style(git_commit *& commit, git_oid const & oid) const
        {
            assert(git_commit_lookup(&commit, repo_, &oid) == 0);
        }

        Commit commit_lookup(git_oid const & oid) const
        {
            git_commit * commit;
            commit_lookup_c_style(commit, oid);
            return Commit(commit);
        }

        int object_lookup(git_object *& object, git_oid const & oid, git_otype type) const 
        {
            return git_object_lookup(&object, repo_, &oid, type);
        }

        int merge_base(git_oid & out, git_oid const * one, git_oid const * two) const
        {
            return git_merge_base(&out, repo_, one, two);
        }

        int revparse(git_revspec & out, const char * spec) const
        {
            return git_revparse(&out, repo_, spec);
        }

        int revparse_single(git_object *& out, const char * spec) const
        {
            return git_revparse_single(&out, repo_, spec);
        }

        std::shared_ptr<RevWalker> rev_walker() const
        {
            git_revwalk * out;
            git_revwalk_new(&out, repo_);
            return std::make_shared<RevWalker>(out);
        }
        
        explicit Repository(std::string const & dir)
        {
            assert(git_repository_open_ext(&repo_, dir.c_str(), 0, NULL) == 0);
        }

        Repository              (Repository const &) = delete;
        Repository& operator =  (Repository const &) = delete; 

        ~Repository()
        {
            git_repository_free(repo_);
        }

    private:
        git_repository * repo_;
    };
}

